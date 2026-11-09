# Mini GPU Runtime

Mini GPU Runtime 是一个基于 C++17 和 CUDA Runtime API 构建的轻量级 GPU Runtime 实验项目，用于熟悉 CUDA Runtime 和 GPU 异步执行机制。

项目涵盖 CPU 任务调度、CUDA 资源管理、GPU 异步任务封装及多 Stream 调度，并通过 Nsight Systems 对 GPU 执行行为和性能特征进行分析。

## 已实现功能

- **TaskQueue / ThreadPool**：基于 mutex、condition_variable 和 future 实现多线程任务执行。
- **GpuBuffer**：使用 RAII 管理 GPU 显存。
- **CudaStream / CudaEvent**：封装 CUDA Stream 和 Event 的创建、同步与生命周期。
- **GpuTask**：封装异步 Memset 操作，通过 Event 查询和等待任务完成。
- **GpuTaskScheduler**：使用 Round-Robin 策略将任务分配到多个 CUDA Stream。

项目重点关注资源所有权、异常安全、异步执行和对象生命周期。

## GPU 多 Stream 性能实验

### 实验配置

- GPU：NVIDIA RTX 4090 24 GiB
- 工作负载：6 个独立的 `cudaMemsetAsync`，每个 Buffer 为 64 MiB
- Stream 数量：1 / 3 / 6
- 测量工具：Nsight Systems
- 每组重复 10 次，通过 SQLite 自动提取 GPU 操作时间戳

GPU Timeline 总跨度定义为：

`MAX(Memset.end) - MIN(Memset.start)`

### 实验结果

| Stream 数量 | GPU Timeline 中位数 | 单个 Memset 耗时中位数 |
|---|---:|---:|
| 1 | 0.4312 ms | 0.0705 ms |
| 3 | 0.4264 ms | 0.0727 ms |
| 6 | 0.4235 ms | 0.0751 ms |

### 分析与发现

1. **多 Stream 不等于线性加速。** Nsight Systems 显示不同 Stream 的 Memset 存在执行重叠，但整体 GPU 时间跨度只略有下降。

2. **单个操作耗时有所增加。** 随着 Stream 数量增加，Memset 耗时中位数从 0.0705 ms 增加到 0.0751 ms，可能与 GPU 资源竞争有关。

3. **任务提交间隔可能限制并发。** 实验中相邻 Memset 的启动时间通常相隔数十微秒，而单个操作耗时也只有几十微秒，因此实际重叠程度有限。

当前结果尚不能区分显存带宽限制和 CPU 提交开销的影响，需要进一步实验验证。

## 构建与运行

需要 C++17、CMake 和 CUDA Toolkit。

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

运行 Scheduler 实验：

```bash
cmake --build build --target profile_scheduler
python3 experiments/run_scheduler_experiment.py
```

实验脚本使用 Nsight Systems 采集数据，生成 SQLite 和 CSV 分析结果。

## 后续计划

- 分离 GPU 资源分配与任务提交，分析 CPU 提交开销
- 使用 Nsight Compute 分析显存带宽利用率
- 探索更接近真实计算场景的 GPU Kernel 调度

---

本项目主要用于学习和实验，并非生产级 GPU Runtime。