#include "thread_pool.h"
#include <cstddef>
#include <stdexcept>

ThreadPool::ThreadPool(std::size_t num_threads) {
    if (num_threads == 0) {
        throw std::invalid_argument("num_threads must be greater than zero");
    }

    workers_.reserve(num_threads);

    try{
        for (std::size_t i = 0; i < num_threads; i++) {
            // 等价于 std::thread worker([this]{worker_loop();}); workers_.push_back(std::move(worker));
            workers_.emplace_back([this] {
                worker_loop();
            });
        }
    } catch (...) {
        queue_.shutdown();
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        throw;
    }
}

ThreadPool::~ThreadPool(){
    shutdown();
}

void ThreadPool::shutdown() {
    // 当前约束：ThreadPool 的 Shutdown 由一个管理线程负责，不能从 Worker 内部调用，也不能由多个线程同时调用。
    queue_.shutdown();
    for (std::thread& worker: workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker_loop() {
    // std::packaged_task 会自动捕获它所执行的任务抛出的异常。wait_and_run() 正常返回 true。
    while (queue_.wait_and_run()) {
    }
}