#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <cstddef>
#include <condition_variable>
#include <future>
#include <memory>
#include <type_traits>
#include <utility>
#include <stdexcept>

class TaskQueue {
public:
    template <typename F>
    auto submit(F&& func)
        -> std::future<std::invoke_result_t<std::decay_t<F>&>>;

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

template <typename F>
auto TaskQueue::submit(F&& func) -> std::future<std::invoke_result_t<std::decay_t<F>&>> {
    using Result = std::invoke_result_t<std::decay_t<F>&>;
    
    auto task_ptr = std::make_shared<std::packaged_task<Result()>>(std::forward<F>(func));

    auto future = task_ptr->get_future();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_) {
            throw std::runtime_error("TaskQueue has shutdown. Submit failed.");
        }
        tasks_.push([task_ptr] {
            (*task_ptr)();
        });
    }
    cv_.notify_one();

    return future;
}