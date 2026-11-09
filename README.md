# mini-gpu-runtime

## Scheduler profiling experiment

Build `profile_scheduler`, ensure the Nsight Systems CLI (`nsys`) is on `PATH`,
then run:

```sh
cmake -S . -B build
cmake --build build --target profile_scheduler
python3 experiments/run_scheduler_experiment.py
```

The script profiles 1, 3, and 6 streams 10 times each. Every run gets its own
`.nsys-rep` and exported SQLite database under a timestamped directory in
`experiments/results/`. It validates that each export contains six Memset
activities, then writes per-run timeline measurements, individual Memset
durations, per-run duration sums and overlap ratios, group summaries, and
pairwise comparisons to CSV files in that directory. Each timeline span is
`MAX(end) - MIN(start)` across the six Memset activities. The overlap ratio is
the sum of the six Memset durations divided by that span. To analyze existing
exports separately, run:

```sh
python3 experiments/analyze_scheduler.py experiments/results/run_<timestamp>
```

The analyzer writes `memsets.csv` (one row per Memset), `run_metrics.csv`
(per-run totals and overlap ratios), `summary.csv` (per-configuration
statistics), and `comparison.csv` (pairwise median and range comparisons).
