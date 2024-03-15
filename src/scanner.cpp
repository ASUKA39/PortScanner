#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>

#include "scanner.h"

void TCPConnectScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 与目标端口建立连接，如果连接成功则端口开放，收到RST则端口关闭
    for (int port : ports) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Error: cannot open socket" << std::endl;
            return;
        }
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            // std::cout << "Port " << port << " is closed" << std::endl;
        } else {
            std::cout << "Port " << port << " is open" << std::endl;
        }
        close(sockfd);
    }
}

void TCPSYNScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送SYN包，收到SYN/ACK则端口开放，收到RST则端口关闭
}

void TCPNULLScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送不带标志位的TCP包，如果没有回复则端口开放，收到RST则端口关闭
}

void TCPFINScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送FIN包，收到RST则端口关闭，收到FIN/ACK则端口开放
}

void UDPScanner::scan(const std::string& ip, const std::vector<int>& ports) {
    // 发送UDP包，收到ICMP Port Unreachable则端口关闭，否则端口开放
}

void ICMPScanner::scan(const std::string& ip, const std::vector<int>& ports) {
}

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
    } else if (type == "ICMP") {
        return new ICMPScanner();
    } else {
        return nullptr;
    }
}