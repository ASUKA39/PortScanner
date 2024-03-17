#pragma once

#include <string>
#include <vector>
#include <mutex>

class Target {
public:
    std::string ip;
    std::vector<int> ports;
    
};

class Result : public Target {
public:
    std::mutex mtx;
};