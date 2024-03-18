#pragma once

#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
// #include <coroutine>

class ThreadPool {
public:
    ThreadPool(int numThreads);
    ~ThreadPool();
    void enqueue(std::function<int()> task);
    bool isAllTaskFinished();
    int getAvailableThread();
    bool isAvailable();
    int getTaskSize();
private:
    std::vector<std::thread> workers;
    std::queue<std::function<int()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<int> availableThread;
    bool stop;
};
