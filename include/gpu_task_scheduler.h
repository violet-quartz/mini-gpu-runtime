#pragma once

#include <cstddef>
#include "cuda_stream.h"

class GpuTask;

class GpuTaskScheduler {
public:
    explicit GpuTaskScheduler(std::size_t num_streams);

    GpuTask submit_memset(std::size_t bytes, int value);

private:
    std::vector<CudaStream> streams_;
    std::size_t next_stream_{0};
};