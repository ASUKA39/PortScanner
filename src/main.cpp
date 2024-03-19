#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sys/resource.h>

#include "threadpool.h"
#include "scanner.h"
#include "dict.h"
#include "target.h"

std::map<std::string, Result> resultMap;
// std::atomic<int> fd_count(5);

static bool isIpInvalid(const std::string& ip) {
    struct in_addr addr;
    return inet_pton(AF_INET, ip.c_str(), &addr) == 0;
}

static int getulimit() {
    struct rlimit rlim;
    if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
        return rlim.rlim_cur;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    int threadNum = std::thread::hardware_concurrency() * 3;
    int ulimit = getulimit();
    // std::cout << "ulimit: " << ulimit << std::endl;
    Dict dict;

    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <ip>(/<mask>)" << " <start port>" << "-<end port>" << " <scan type>" << std::endl;
        return 1;
    }
    std::vector<std::string> ipVec;
    std::string ipArg = argv[1];

    int startPort;
    int endPort;
    std::string portRange = argv[2];
    if (portRange.find('-') != std::string::npos) {
        if (portRange.find('-') == 0 || portRange.find('-') == portRange.size() - 1) {
            std::cerr << "Invalid port range" << std::endl;
            return 1;
        }
        startPort = std::stoi(portRange.substr(0, portRange.find('-')));
        startPort = startPort < 1 ? 1 : startPort;
        endPort = std::stoi(portRange.substr(portRange.find('-') + 1));
        if (endPort < startPort) {
            std::cerr << "Invalid port range" << std::endl;
            return 1;
        }
    } else {
        startPort = std::stoi(portRange);
        endPort = startPort;
        if (startPort < 1) {
            std::cerr << "Invalid port number" << std::endl;
            return 1;
        }
    }

    std::string scanType = argc == 5 ? argv[3] : "TCPConnect";
    // Starting Scan at YYYY-MM-DD HH:MM CST
    auto start = std::chrono::high_resolution_clock::now();
    time_t date = std::chrono::system_clock::to_time_t(start);
    std::cout << "Starting Scan at " << std::put_time(std::localtime(&date), "%Y-%m-%d %H:%M %Z") << std::endl;
    
    if (ipArg.find('/') != std::string::npos) {
        std::string ip = ipArg.substr(0, ipArg.find('/'));
        if (isIpInvalid(ip)) {
            std::cerr << "Invalid IP address" << std::endl;
            return 1;
        }
        std::string mask = ipArg.substr(ipArg.find('/') + 1);
        if (std::stoi(mask) < 0 || std::stoi(mask) > 32) {
            std::cerr << "Invalid mask" << std::endl;
            return 1;
        }     

        int maskNum = std::stoi(mask);
        int ipNum = 1 << (32 - maskNum);

        // transform ip to a number
        struct in_addr addr;
        inet_pton(AF_INET, ip.c_str(), &addr);
        uint32_t ipNumb = ntohl(addr.s_addr);

        // reset the last <mask> bits to 0
        ipNumb &= 0xFFFFFFFF << (32 - maskNum);

        for (int i = 0; i < ipNum; i++) {
            addr.s_addr = htonl(ipNumb + i);
            std::string ipWithMask = inet_ntoa(addr);

            ipVec.push_back(ipWithMask);
            // std::cout << ipWithMask << std::endl;
        }
    } else {
        if (isIpInvalid(ipArg)) {
            std::cerr << "Invalid IP address" << std::endl;
            return 1;
        }
        ipVec.push_back(ipArg);
    }

    int taskNum = ipVec.size() * (endPort - startPort + 1);
    int taskSize = taskNum / threadNum + 1 > (ulimit - 3) / (threadNum + 1) 
                    ? (ulimit - 3) / (threadNum + 1)
                    : taskNum / threadNum + 1;
    if (taskNum < threadNum) {
        taskSize = taskNum;
    }

    int count = 0;
    std::vector<Target> targetPack;
    std::vector<std::vector<Target>> targetPacks;
    for (std::string ip : ipVec) {
        for (int i = 0; i < endPort - startPort + 1; i++) {
            if (targetPack.empty()) {
                targetPack.push_back(Target());
                targetPack.back().ip = ip;
            }
            if (ip == targetPack.back().ip) {
                targetPack.back().ports.push_back(startPort + i);
                // std::cout << ip << " " << startPort + i << std::endl;
            } else {
                targetPack.push_back(Target());
                targetPack.back().ip = ip;
                targetPack.back().ports.push_back(startPort + i);
                // std::cout << ip << " " << startPort + i << std::endl;
            }
            count++;
            if (count >= taskSize || (ip == ipVec.back() && i == endPort - startPort)) {
                targetPacks.push_back(targetPack);
                targetPack.clear();
                count = 0;
            }
        }
    }

    {
        ThreadPool pool(threadNum > targetPacks.size() 
                        ? targetPacks.size() 
                        : threadNum);
        for (std::vector<Target> targetPack : targetPacks) {
            pool.enqueue([targetPack, scanType] {
                ScannerFactory factory;
                Scanner* scanner = factory.createScanner(scanType);
                return scanner->scan(targetPack);
            });
        }
    }

    int hostCount = 0;
    for (std::string ip : ipVec) {
        std::sort(resultMap[ip].ports.begin(), resultMap[ip].ports.end());
        if (resultMap[ip].ports.empty()) {
            continue;
        }

        std::cout << "scan report for " << ip << std::endl;
        std::cout << "Not shown: " << (endPort - startPort + 1 - resultMap[ip].ports.size()) << " closed ports" << std::endl;
        if (!resultMap[ip].ports.empty()) {
            hostCount++;
            std::cout << "PORT      STATE SERVICE" << std::endl;
            for (int port : resultMap[ip].ports) {
                std::string mode;
                if (scanType[0] == 'T') {
                    mode = "/tcp";
                } else {
                    mode = "/udp";
                }
                mode = std::to_string(port) + mode;
                printf("%-10s%-6s%-8s\n", mode.c_str(), "open", dict.getServiceByPort(port).c_str());
            }
        }
        std::cout << std::endl;
    }

    // end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    //  Scan completed in 0.00 seconds
    std::cout << "Scan done: " << ipVec.size() 
                << " IP address" << " (" << hostCount << " hosts up)"   // still need to be fixed, ip should be tested first
                << " scanned in " << std::fixed << std::setprecision(2) << elapsed.count() 
                << " seconds" << std::endl;

    return 0;
}