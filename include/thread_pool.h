#pragma once

#include "task_queue.h"
#include <cstddef>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t num_threads);
    ~ThreadPool();

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;

    ThreadPool(ThreadPool&& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;

    template <typename F>
    auto submit(F&& func) {
        return queue_.submit(std::forward<F>(func));
    }

    void shutdown();
private:
    void worker_loop();
    
    TaskQueue queue_;
    std::vector<std::thread> workers_;
};
