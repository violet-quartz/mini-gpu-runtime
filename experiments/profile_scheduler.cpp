#include <cstddef>
#include <charconv>
#include <exception>
#include <iostream>
#include <string_view>
#include <vector>

#include "gpu_task.h"
#include "gpu_task_scheduler.h"

int main(int argc, char* argv[]) {
    std::size_t num_streams = 3;
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [num_streams]\n";
        return 1;
    }
    if (argc == 2) {
        const std::string_view arg(argv[1]);
        const auto result = std::from_chars(
            arg.data(), arg.data() + arg.size(), num_streams);
        if (result.ec != std::errc{} ||
            result.ptr != arg.data() + arg.size() || num_streams == 0) {
            std::cerr << "num_streams must be a positive integer within size_t range\n";
            return 1;
        }
    }

    try {
        GpuTaskScheduler scheduler{num_streams};
        std::size_t task_cnt = 6;
        std::size_t buffer_size = 64 * 1024 * 1024; // 64 MiB
        std::vector<GpuTask> tasks;
        tasks.reserve(task_cnt);
        for(std::size_t i = 0; i < task_cnt; i++) {
            tasks.emplace_back(scheduler.submit_memset(buffer_size, 0x30 + i));
        }
        for(std::size_t i = 0; i < task_cnt; i++) {
            tasks[i].wait();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
