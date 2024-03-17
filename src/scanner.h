#pragma once

#include <iostream>
#include <vector>

#include "target.h"

class Scanner {
public:
    // virtual void scan(const std::string& ip, const std::vector<int>& ports) = 0;
    virtual int scan(const std::vector<Target>& targets) = 0;
};

class ScannerFactory {
public:
    Scanner* createScanner(const std::string& type);
};

class TCPConnectScanner : public Scanner {
public:
    int scan(const std::vector<Target>& targets);
};

class TCPSYNScanner : public Scanner {
public:
    int scan(const std::vector<Target>& targets);
};

class TCPNULLScanner : public Scanner {
public:
    int scan(const std::vector<Target>& targets);
};

class TCPFINScanner : public Scanner {
public:
    int scan(const std::vector<Target>& targets);
};

class UDPScanner : public Scanner {
public:
    int scan(const std::vector<Target>& targets);
};

// class ICMPScanner : public Scanner {
// public:
//     void scan(const std::vector<Target>& targets);
// };
