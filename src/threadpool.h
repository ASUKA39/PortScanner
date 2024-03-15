#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <coroutine>

class ThreadPool {
public:
    ThreadPool();

    ~ThreadPool();

    template<typename Func, typename... Args>
    auto enqueue(Func &&func, Args &&... args);

private:
    std::vector<std::thread> threads;
    std::vector<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool stop = false;

    void threadFunction(int cpu_core);

    void coroutineFunction();
};

#endif // __THREADPOOL_H__