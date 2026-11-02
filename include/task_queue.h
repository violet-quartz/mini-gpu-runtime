#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <cstddef>
#include <condition_variable>

class TaskQueue {
public:
    bool submit(std::function<void()> task);
    bool run_one();
    bool wait_and_run();
    void shutdown();
    std::size_t pending() const;
private:
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopping_{false};
};