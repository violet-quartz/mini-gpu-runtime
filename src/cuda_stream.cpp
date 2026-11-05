#include "cuda_stream.h"

#include <stdexcept>

CudaStream::CudaStream() {
    cudaError_t err = cudaStreamCreateWithFlags(
        &stream_, cudaStreamNonBlocking
    );

    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

CudaStream::~CudaStream() {
    if (stream_) {
        cudaStreamDestroy(stream_);
    }
}

CudaStream::CudaStream(CudaStream&& other) noexcept
    : stream_(other.stream_)  {
    other.stream_ = nullptr;
}

CudaStream& CudaStream::operator=(CudaStream&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (stream_) {
        cudaStreamDestroy(stream_);
    }
    stream_ = other.stream_;
    other.stream_ = nullptr;
    return *this;
}

cudaStream_t CudaStream::get() const noexcept {
    return stream_;
}

void CudaStream::synchronize() const {
    if (stream_ == nullptr) {
        throw std::logic_error("Cannot synchronize an empty CudaStream.");
    }
    cudaError_t err = cudaStreamSynchronize(stream_);
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}
