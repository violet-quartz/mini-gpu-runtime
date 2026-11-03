#include "gpu_buffer.h"
#include <cuda_runtime.h>


GpuBuffer::GpuBuffer(std::size_t bytes) {
    if (bytes == 0) {
        return;
    }

    cudaError_t err = cudaMalloc(&ptr_, bytes_);

    if (err != cudaSuccess) {
        throw std::runtime_error(
            cudaGetErrorString(err);
        );
    }
    bytes_ = bytes;
}

GpuBuffer::~GpuBuffer() {
    if (ptr_ != nullptr) {
        cudaFree(ptr_);
    }
}

GpuBuffer::GpuBuffer(GpuBuffer&& other) noexcept
    : ptr_(other.ptr_), bytes_(other.bytes_) {
    other.ptr_ = nullptr;
    other.bytes_ = 0;
}

GpuBuffer::GpuBuffer& operator=(GpuBuffer&& other) noexcept
{
    if (this == &other) {
        return *this;
    }
    cudaFree(ptr_);
    ptr_ = other.ptr_;
    bytes_ = other.bytes_;
    other.ptr_ = nullptr;
    other.bytes_ = 0;
    return *this;
}

void* GpuBuffer::data() const noexcept {
    return ptr_;
}

std::size_t GpuBuffer::size() const noexcept {
    return bytes_;
}