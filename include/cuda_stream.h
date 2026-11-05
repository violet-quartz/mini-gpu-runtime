#pragma once

#include <cuda_runtime.h>

class CudaStream {
public:
    CudaStream();
    ~CudaStream();

    CudaStream(const CudaStream&) = delete;
    CudaStream& operator=(const CudaStream&) = delete;

    CudaStream(CudaStream&& other) noexcept;
    CudaStream& operator=(CudaStream&& other) noexcept;

    cudaStream_t get() const noexcept;
    void synchronize() const;
private:
    cudaStream_t stream_{nullptr};
};