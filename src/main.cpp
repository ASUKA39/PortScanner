#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "threadpool.h"
#include "scanner.h"
#include "dict.h"

std::vector<int> result;
std::mutex resultMutex;

int main(int argc, char* argv[]) {
    int threadNum = std::thread::hardware_concurrency() * 3;
    Dict dict;

    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <ip>" << " <start port>" << "-<end port>" << " <scan type>" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <ip>/<mask>" << " <start port>" << "-<end port>" << " <scan type>" << std::endl;
        return 1;
    }
    std::vector<std::string> ipVec;
    std::string ipArg = argv[1];

    // int startPort = std::stoi(argv[2]);
    // int endPort = std::stoi(argv[3]);
    int startPort;
    int endPort;
    std::string portRange = argv[2];
    if (portRange.find('-') != std::string::npos) {
        startPort = std::stoi(portRange.substr(0, portRange.find('-')));
        endPort = std::stoi(portRange.substr(portRange.find('-') + 1));
    } else {
        startPort = std::stoi(portRange);
        endPort = startPort;
    }

    std::string scanType = argc == 5 ? argv[3] : "TCPConnect";
    // Starting Scan at YYYY-MM-DD HH:MM CST
    std::cout << "Starting Scan at " << __DATE__ << " " << __TIME__ << " CST" << std::endl;
    // start time
    auto start = std::chrono::high_resolution_clock::now();
    
    if (ipArg.find('/') != std::string::npos) {
        std::string ip = ipArg.substr(0, ipArg.find('/'));
        std::string mask = ipArg.substr(ipArg.find('/') + 1);        

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
        ipVec.push_back(ipArg);
    }

    for (std::string ip : ipVec) {
        // scan report for <ip>
        std::cout << "scan report for " << ip << std::endl;

        {
            ThreadPool pool(threadNum);
            for (int i = 0; i < threadNum; i++) {
                std::vector<int> subPorts;
                for (int j = startPort + i; j <= endPort; j += threadNum) {
                    subPorts.push_back(j);
                }
                pool.enqueue([i, ip, subPorts, scanType] {
                    ScannerFactory factory;
                    Scanner* scanner = factory.createScanner(scanType);
                    scanner->scan(ip, subPorts);
                });
            }

            // while (!pool.isAllTaskFinished()) {
            //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // }
        }

        std::cout << "Not shown: " << (endPort - startPort + 1 - result.size()) << " closed ports" << std::endl;

        if (!result.empty()) {
            std::cout << "PORT      STATE SERVICE" << std::endl;
            std::sort(result.begin(), result.end());
            for (int port : result) {
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
        result.clear();
        std::cout << std::endl;
    }

    // end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    //  Scan completed in 0.00 seconds
    std::cout << "Scan completed in " << std::fixed << std::setprecision(2) << elapsed.count() << " seconds" << std::endl;

    return 0;
}