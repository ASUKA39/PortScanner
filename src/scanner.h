#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <iostream>
#include <vector>

class Scanner {
public:
    virtual void scan(const std::string& ip, const std::vector<int>& ports) = 0;
};

class ScannerFactory {
public:
    Scanner* createScanner(const std::string& type);
};

class TCPConnectScanner : public Scanner {
public:
    void scan(const std::string& ip, const std::vector<int>& ports);
};

class TCPSYNScanner : public Scanner {
public:
    void scan(const std::string& ip, const std::vector<int>& ports);
};

class TCPNULLScanner : public Scanner {
public:
    void scan(const std::string& ip, const std::vector<int>& ports);
};

class TCPFINScanner : public Scanner {
public:
    void scan(const std::string& ip, const std::vector<int>& ports);
};

class UDPScanner : public Scanner {
public:
    void scan(const std::string& ip, const std::vector<int>& ports);
};

class ICMPScanner : public Scanner {
public:
    void scan(const std::string& ip, const std::vector<int>& ports);
};

#endif // __SCANNER_H__