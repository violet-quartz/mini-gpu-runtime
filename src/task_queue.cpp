#include "task_queue.h"
#include <utility>

bool TaskQueue::submit(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_) {
            return false;
        }
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
    return true;
}

bool TaskQueue::run_one() {
    std::function<void()> task;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tasks_.empty()) {
            return false;
        }
        task = std::move(tasks_.front());
        tasks_.pop();
    }
    task();
    return true;
}

bool TaskQueue::wait_and_run() {
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] {
            return stopping_ || !tasks_.empty();
        });
        if (tasks_.empty() && stopping_) {
            return false;
        }
        task = std::move(tasks_.front());
        tasks_.pop();
    }
    task();
    return true;
}

void TaskQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    cv_.notify_all();
}

std::size_t TaskQueue::pending() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}