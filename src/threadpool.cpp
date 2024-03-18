#include <functional>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sched.h>

#include "threadpool.h"

ThreadPool::ThreadPool(int numThreads) : stop(false) {
    availableThread = 0;
    int numCore = std::thread::hardware_concurrency();
    for (int i = 0; i < numThreads; i++) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i % numCore, &cpuset);
        workers.emplace_back([this, cpuset] {
            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
            while(true) {
                availableThread++;
                std::function<int()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    if (this->stop && this->tasks.empty()) {
                        return;
                    }
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    availableThread--;
                }
                int err = task();
                if (err < 0) {
                    std::cerr << "Error from thread pool: task failed with error " << err << std::endl;
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<int()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(task);
    }
    condition.notify_one();
}

bool ThreadPool::isAllTaskFinished() {
    std::unique_lock<std::mutex> lock(queueMutex);
    return tasks.empty() && availableThread == workers.size();
}

bool ThreadPool::isAvailable() {
    std::unique_lock<std::mutex> lock(queueMutex);
    return availableThread > 0;
}

int ThreadPool::getTaskSize() {
    return tasks.size();
}

int ThreadPool::getAvailableNum() {
    return availableThread.load();
}