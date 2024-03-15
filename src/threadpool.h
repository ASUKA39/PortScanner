#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

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
    void enqueue(std::function<void()> task);
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

#endif // __THREADPOOL_H__