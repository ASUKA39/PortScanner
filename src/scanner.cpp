#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "scanner.h"
#include "target.h"

extern std::map<std::string, Result> resultMap;
// extern std::atomic<int> fd_count;

int TCPConnectScanner::scan(const std::vector<Target>& targets) {
    // 与目标端口建立连接，如果连接成功则端口开放，收到RST则端口关闭
    // using epoll to scan: slower but if ip is unreachable, non-blocking I/O can handle it
    std::map<int, struct sockaddr_in> addrMap;
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        std::cerr << "Error: cannot create epoll " << epollfd << std::endl;
        return -1;
    }
    // fd_count++;

    struct epoll_event ev;
    int nfds, sockfd;
    std::map<int, bool> socketMap;
    for (Target target : targets) {
        std::string ip = target.ip;
        for (int port : target.ports) {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                std::cerr << "Error: cannot open socket" << std::endl;
                std::cerr << "errno: " << errno << std::endl;
                // std::cerr << "fd_count: " << fd_count.load() << std::endl;
                close(epollfd);
                return -1;
            }
            // fd_count++;

            fcntl(sockfd, F_SETFL, O_NONBLOCK); // set non-blocking
            struct sockaddr_in addr;

            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            addr.sin_port = htons(port);
            addrMap[sockfd] = addr;

            if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                if (errno != EINPROGRESS) { // not in progress
                    std::cerr << "Error: cannot connect" << std::endl;
                    close(sockfd);
                    close(epollfd);
                    return -1;
                }
            }
            socketMap[sockfd] = true;

            ev.events = EPOLLOUT;
            ev.data.fd = sockfd;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
        }
    }

    // wait for events
    struct epoll_event events[20];
    int socketCount = socketMap.size();
    while (true) {
        nfds = epoll_wait(epollfd, events, 16, 1000);
        if (nfds < 0) {
            std::cerr << "Error: epoll_wait" << std::endl;
            close(epollfd);
            return -1;
        } else if (nfds == 0 || socketCount <= 0) {
            for (auto it = socketMap.begin(); it != socketMap.end(); it++) {
                if (it->second) {
                    close(it->first);
                    it->second = false;
                    // fd_count--;
                }
            }
            break;
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLOUT) {  // connect might not be success, need to check
                int error;
                socklen_t len = sizeof(error);
                getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &error, &len);
                if (error == 0) {   // no error, connect success
                    std::string ip = inet_ntoa(addrMap[events[i].data.fd].sin_addr);
                    std::lock_guard<std::mutex> lock(resultMap[ip].mtx);
                    struct sockaddr_in addr = addrMap[events[i].data.fd];
                    resultMap[ip].ports.push_back(ntohs(addr.sin_port));
                }
            }
            socketMap[events[i].data.fd] = false;
            close(events[i].data.fd);
            socketCount--;
            // fd_count--;
        }
    }
    close(epollfd);
    // fd_count--;
    return 0;
}

int TCPSYNScanner::scan(const std::vector<Target>& targets) {
    // 发送SYN包，收到SYN/ACK则端口开放，收到RST则端口关闭
    return 0;
}

int TCPNULLScanner::scan(const std::vector<Target>& targets) {
    // 发送不带标志位的TCP包，如果没有回复则端口开放，收到RST则端口关闭
    return 0;
}

int TCPFINScanner::scan(const std::vector<Target>& targets) {
    // 发送FIN包，收到RST则端口关闭，收到FIN/ACK则端口开放
    return 0;
}

int UDPScanner::scan(const std::vector<Target>& targets) {
    // 发送UDP包，收到ICMP Port Unreachable则端口关闭，否则端口开放
    return 0;
}

// void ICMPScanner::scan(const std::string& ip, const std::vector<int>& ports) {
// }

Scanner* ScannerFactory::createScanner(const std::string& type) {
    if (type == "TCPConnect") {
        return new TCPConnectScanner();
    } else if (type == "TCPSYN") {
        return new TCPSYNScanner();
    } else if (type == "TCPNULL") {
        return new TCPNULLScanner();
    } else if (type == "TCPFIN") {
        return new TCPFINScanner();
    } else if (type == "UDP") {
        return new UDPScanner();
    // } else if (type == "ICMP") {
    //     return new ICMPScanner();
    } else {
        return nullptr;
    }
}
