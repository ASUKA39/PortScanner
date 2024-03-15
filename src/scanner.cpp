#include <iostream>
#include <vector>

#include "scanner.h"

void TCPConnectScanner::scan(const std::string& ip, const std::vector<int>& ports) {

}

void TCPSYNScanner::scan(const std::string& ip, const std::vector<int>& ports) {

}

void TCPNULLScanner::scan(const std::string& ip, const std::vector<int>& ports) {

}

void TCPFINScanner::scan(const std::string& ip, const std::vector<int>& ports) {

}

void UDPScanner::scan(const std::string& ip, const std::vector<int>& ports) {

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