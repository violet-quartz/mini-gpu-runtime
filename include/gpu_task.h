#pragma once

#include <cstddef>
#include "cuda_event.h"
#include "gpu_buffer.h"

class CudaStream;

class GpuTask {
public:
    explicit GpuTask(std::size_t bytes);
    ~GpuTask() noexcept;

    GpuTask(const GpuTask&) = delete;
    GpuTask& operator=(const GpuTask&) = delete;

    GpuTask(GpuTask&& other) noexcept;
    GpuTask& operator=(GpuTask&& other);

    void submit_memset(CudaStream& stream, int value);

    bool query() const;
    void wait() const;

    void* data() const noexcept;
    std::size_t size() const noexcept;

private:
    GpuBuffer buffer_;
    CudaEvent event_;
    bool submitted_{false};
};