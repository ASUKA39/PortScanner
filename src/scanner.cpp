#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "scanner.h"

extern std::vector<int> result;
extern std::mutex resultMutex;

void TCPConnectScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 与目标端口建立连接，如果连接成功则端口开放，收到RST则端口关闭
    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd < 0) {
    //     std::cerr << "Error: cannot open socket" << std::endl;
    //     return;
    // }
    // struct sockaddr_in addr;
    // addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // for (int port : ports) {
    //     addr.sin_port = htons(port);
    //     if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) >= 0) {
    //         // std::cout << "Port " << port << " is open" << std::endl;
    //         std::lock_guard<std::mutex> lock(resultMutex);
    //         result.push_back(port);
    //     }
    // }
    // close(sockfd);
    // return;

    // using epoll to scan
    std::map<int, struct sockaddr_in> addrMap;
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        std::cerr << "Error: cannot create epoll" << std::endl;
        return;
    }

    struct epoll_event ev, events[ports.size()+1];
    int nfds, sockfd;
    for (int port : ports) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Error: cannot open socket" << std::endl;
            return;
        }
        fcntl(sockfd, F_SETFL, O_NONBLOCK); // set non-blocking
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port = htons(port);
        addrMap[sockfd] = addr;

        if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            if (errno != EINPROGRESS) {
                std::cerr << "Error: cannot connect" << std::endl;
                return;
            }
        }

        ev.events = EPOLLOUT;
        ev.data.fd = sockfd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
    }

    // wait for events
    while (true) {
        nfds = epoll_wait(epollfd, events, 16, 1);
        if (nfds < 0) {
            std::cerr << "Error: epoll_wait" << std::endl;
            return;
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLOUT) {  // connect might not be success, need to check
                int error;
                socklen_t len = sizeof(error);
                getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &error, &len);
                if (error == 0) {   // no error, connect success
                    std::lock_guard<std::mutex> lock(resultMutex);
                    struct sockaddr_in addr = addrMap[events[i].data.fd];
                    result.push_back(ntohs(addr.sin_port));
                }
                close(events[i].data.fd);
            }
        }
        if (nfds == 0) {
            break;
        }
    }
    close(epollfd);
    return;
}

void TCPSYNScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送SYN包，收到SYN/ACK则端口开放，收到RST则端口关闭
}

void TCPNULLScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送不带标志位的TCP包，如果没有回复则端口开放，收到RST则端口关闭
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error: cannot open socket" << std::endl;
        return;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    for (int port : ports) {
        addr.sin_port = htons(port);

    }

    close(sockfd);
    return;
}

void TCPFINScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送FIN包，收到RST则端口关闭，收到FIN/ACK则端口开放
}

void UDPScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送UDP包，收到ICMP Port Unreachable则端口关闭，否则端口开放
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