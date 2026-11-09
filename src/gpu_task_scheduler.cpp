#include "gpu_task_scheduler.h"
#include "gpu_task.h"

#include <cstddef>
#include <stdexcept>
#include <vector>

GpuTaskScheduler::GpuTaskScheduler(std::size_t num_streams) {
    if (num_streams == 0) {
        throw std::invalid_argument("The number of streams should be greater than 0");
    }
    streams_.reserve(num_streams);
    for (std::size_t i = 0; i < num_streams; i++) {
        streams_.emplace_back();
    }
}

GpuTask GpuTaskScheduler::submit_memset(std::size_t bytes, int value) {
    GpuTask task{bytes};
    task.submit_memset(streams_[next_stream_], value);
    next_stream_ = (next_stream_ + 1) % streams_.size();
    return task;
}
