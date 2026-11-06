#pragma once

#include <cuda_runtime.h>

class CudaStream;

class CudaEvent {
public:
    explicit CudaEvent(unsigned int flags = cudaEventDefault);
    ~CudaEvent();

    CudaEvent(const CudaEvent&) = delete;
    CudaEvent& operator=(const CudaEvent&) = delete;

    CudaEvent(CudaEvent&& other) noexcept;
    CudaEvent& operator=(CudaEvent&& other) noexcept;

    cudaEvent_t get() const noexcept;

    void record(const CudaStream& stream);
    bool query() const;
    void synchronize() const;

private:
    cudaEvent_t event_{nullptr};
};