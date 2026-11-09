#include <cstddef>
#include <vector>

#include "gpu_task.h"
#include "gpu_task_scheduler.h"
#include "test_helpers.h"

namespace {

void test_zero_streams() {
    expect_logic_error([] { GpuTaskScheduler scheduler(0); });
}

void test_single_stream() {
    GpuTaskScheduler scheduler(1);
    std::vector<GpuTask> tasks;
    tasks.reserve(7);
    for (std::size_t i = 0; i < 7; ++i) {
        tasks.emplace_back(scheduler.submit_memset(1024 + i, 0x30 + i));
    }
    require(tasks.size() == 7);
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        tasks[i].wait();
        require(tasks[i].query());
        require(tasks[i].size() == 1024 + i);
        check_bytes(tasks[i], static_cast<unsigned char>(0x30 + i));
    }
}

void test_multiple_streams() {
    GpuTaskScheduler scheduler(3);
    std::vector<GpuTask> tasks;
    tasks.reserve(7);
    for (std::size_t i = 0; i < 7; ++i) {
        tasks.emplace_back(scheduler.submit_memset(1024 + i, 0xA0 + i));
    }
    require(tasks.size() == 7);
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        tasks[i].wait();
        require(tasks[i].query());
        require(tasks[i].size() == 1024 + i);
        check_bytes(tasks[i], static_cast<unsigned char>(0xA0 + i));
    }
}

void test_task_outlives_scheduler() {
    GpuTask task = [] {
        GpuTaskScheduler scheduler(2);
        return scheduler.submit_memset(1024, 0xAB);
    }();
    task.wait();
    require(task.query());
    require(task.size() == 1024);
    check_bytes(task, 0xAB);
}

void test_zero_size_task() {
    GpuTaskScheduler scheduler(3);
    expect_logic_error([&] { (void)scheduler.submit_memset(0, 0xAB); });

    // A rejected submission must leave the scheduler usable.
    auto task = scheduler.submit_memset(1024, 0x5A);
    task.wait();
    require(task.query());
    check_bytes(task, 0x5A);
}

} // namespace

int main() {
    run_test("test_zero_streams", test_zero_streams);
    run_test("test_single_stream", test_single_stream);
    run_test("test_multiple_streams", test_multiple_streams);
    run_test("test_task_outlives_scheduler", test_task_outlives_scheduler);
    run_test("test_zero_size_task", test_zero_size_task);
}
