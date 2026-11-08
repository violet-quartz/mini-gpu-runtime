#include "gpu_task.h"
#include "gpu_buffer.h"
#include "cuda_event.h"

#include <cuda_runtime.h>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <exception>
#include <cstddef>


GpuTask::GpuTask(std::size_t bytes): buffer_(bytes) {
}

GpuTask::GpuTask(GpuTask&& other) noexcept
    : buffer_(std::move(other.buffer_)), 
      event_(std::move(other.event_)),
      submitted_(other.submitted_) {
        other.submitted_ = false;
}

GpuTask& GpuTask::operator=(GpuTask&& other) {
    if (this == &other) {
        return *this;
    }
    if (submitted_) {
        event_.synchronize();
    }
    buffer_ = std::move(other.buffer_);
    event_ = std::move(other.event_);
    submitted_ = other.submitted_;
    other.submitted_ = false;
    return *this;
}

GpuTask::~GpuTask() noexcept {
    if (submitted_) {
        try {
            event_.synchronize();
        } catch (const std::exception& e) {
            std::cerr << "Event synchronize exception: " << e.what() << std::endl;
            std::terminate();
        }
    }
}

void GpuTask::submit_memset(CudaStream& stream, int value) {
    if (submitted_) {
        throw std::logic_error("GpuTask already submitted");
    }
    if (stream.get() == nullptr) {
        throw std::logic_error("Cannot submit to an empty CudaStream");
    }
    if (buffer_.data() == nullptr) {
        throw std::logic_error("Cannot submit with an empty GpuBuffer");
    }
    cudaError_t err = cudaMemsetAsync(
        buffer_.data(),
        value,
        buffer_.size(),
        stream.get()
    );
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
    try {
        event_.record(stream);
    } catch (...) {
        try {
            stream.synchronize();
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    
    submitted_ = true;
}

bool GpuTask::query() const {
    if (!submitted_) {
        throw std::logic_error("Cannot query unsubmitted task");
    }
    return event_.query();
}

void GpuTask::wait() const {
    if (!submitted_) {
        throw std::logic_error("Cannot wait for an unsubmitted task");
    }
    event_.synchronize();
}

void* GpuTask::data() const noexcept {
    return buffer_.data();
}

std::size_t GpuTask::size() const noexcept {
    return buffer_.size();
}

