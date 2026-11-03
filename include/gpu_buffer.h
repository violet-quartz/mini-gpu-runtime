#pragma once

#include <cstddef>

class GpuBuffer{
public:
    explicit GpuBuffer(std::size_t bytes);
    ~GpuBuffer();

    GpuBuffer(const GpuBuffer& other) = delete;
    GpuBuffer& operator=(const GpuBuffer& other) = delete;

    GpuBuffer(GpuBuffer&& other) noexcept;
    GpuBuffer& operator=(GpuBuffer&& other) noexcept;

    void* data() const noexcept;
    std::size_t size() const noexcept;

private:
    void* ptr_{nullptr};
    std::size_t bytes_{0};
};