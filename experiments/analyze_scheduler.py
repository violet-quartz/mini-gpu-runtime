import argparse
import csv
import re
import sqlite3
import statistics
from contextlib import closing
from pathlib import Path


PROFILE_NAME = re.compile(r"streams_(1|3|6)_run_(\d{2})\.sqlite$")
LEGACY_NAME = re.compile(r"scheduler_(1|3|6)\.sqlite$")
STREAM_COUNTS = (1, 3, 6)
EXPECTED_MEMSETS = 6


def analyze_profiles(input_dir):
    input_dir = Path(input_dir)
    profiles = []
    for path in sorted(input_dir.glob("*.sqlite")):
        match = PROFILE_NAME.fullmatch(path.name)
        if match:
            streams, run = map(int, match.groups())
        else:
            match = LEGACY_NAME.fullmatch(path.name)
            if not match:
                continue
            streams = int(match.group(1))
            run = 1

        with closing(sqlite3.connect(path)) as connection:
            rows = connection.execute(
                """
                SELECT start, end, streamId
                FROM CUPTI_ACTIVITY_KIND_MEMSET
                ORDER BY start
                """
            ).fetchall()

        if len(rows) != EXPECTED_MEMSETS:
            raise ValueError(
                f"{path}: expected {EXPECTED_MEMSETS} Memset activities, found {len(rows)}"
            )

        start = min(row[0] for row in rows)
        end = max(row[1] for row in rows)
        span_ns = end - start
        if span_ns <= 0:
            raise ValueError(f"{path}: invalid Memset GPU timeline span: {span_ns} ns")

        memsets = [
            {
                "streams": streams,
                "run": run,
                "sqlite": path.name,
                "task": task,
                "stream_id": stream_id,
                "start_ms": (memset_start - start) / 1_000_000,
                "duration_ns": memset_end - memset_start,
                "duration_ms": (memset_end - memset_start) / 1_000_000,
            }
            for task, (memset_start, memset_end, stream_id) in enumerate(rows)
        ]
        if any(memset["duration_ns"] < 0 for memset in memsets):
            raise ValueError(f"{path}: found Memset activity with end before start")

        duration_sum_ns = sum(memset["duration_ns"] for memset in memsets)
        profiles.append(
            {
                "streams": streams,
                "run": run,
                "sqlite": path.name,
                "span_ns": span_ns,
                "span_ms": span_ns / 1_000_000,
                "duration_sum_ns": duration_sum_ns,
                "duration_sum_ms": duration_sum_ns / 1_000_000,
                "mean_duration_ms": duration_sum_ns / EXPECTED_MEMSETS / 1_000_000,
                "overlap_ratio": duration_sum_ns / span_ns,
                "memsets": memsets,
            }
        )

    if not profiles:
        raise FileNotFoundError(f"No scheduler profile SQLite files found in {input_dir}")

    profiles.sort(key=lambda profile: (profile["streams"], profile["run"]))
    run_metrics = [
        {
            key: profile[key]
            for key in (
                "streams",
                "run",
                "sqlite",
                "span_ns",
                "span_ms",
                "duration_sum_ns",
                "duration_sum_ms",
                "mean_duration_ms",
                "overlap_ratio",
            )
        }
        for profile in profiles
    ]
    memsets = [
        memset
        for profile in profiles
        for memset in profile["memsets"]
    ]

    summaries = []
    for streams in STREAM_COUNTS:
        group_profiles = [
            profile
            for profile in profiles
            if profile["streams"] == streams
        ]
        if not group_profiles:
            continue
        group_memsets = [memset for profile in group_profiles for memset in profile["memsets"]]
        operation_durations = [memset["duration_ms"] for memset in group_memsets]
        spans = [profile["span_ms"] for profile in group_profiles]
        duration_sums = [profile["duration_sum_ms"] for profile in group_profiles]
        overlap_ratios = [profile["overlap_ratio"] for profile in group_profiles]
        summaries.append(
            {
                "streams": streams,
                "runs": len(group_profiles),
                "operations": len(group_memsets),
                "span_median_ms": statistics.median(spans),
                "span_min_ms": min(spans),
                "span_max_ms": max(spans),
                "operation_mean_ms": statistics.mean(operation_durations),
                "operation_median_ms": statistics.median(operation_durations),
                "operation_min_ms": min(operation_durations),
                "operation_max_ms": max(operation_durations),
                "duration_sum_median_ms": statistics.median(duration_sums),
                "duration_sum_min_ms": min(duration_sums),
                "duration_sum_max_ms": max(duration_sums),
                "overlap_ratio_median": statistics.median(overlap_ratios),
                "overlap_ratio_min": min(overlap_ratios),
                "overlap_ratio_max": max(overlap_ratios),
            }
        )

    comparisons = []
    for left_index, left in enumerate(summaries):
        for right in summaries[left_index + 1 :]:
            span_delta_ms = right["span_median_ms"] - left["span_median_ms"]
            operation_delta_ms = (
                right["operation_median_ms"] - left["operation_median_ms"]
            )
            comparisons.append(
                {
                    "streams_a": left["streams"],
                    "streams_b": right["streams"],
                    "span_median_delta_ms_b_minus_a": span_delta_ms,
                    "operation_median_delta_ms_b_minus_a": operation_delta_ms,
                    "operation_median_delta_percent_vs_a": (
                        operation_delta_ms / left["operation_median_ms"] * 100
                        if left["operation_median_ms"]
                        else ""
                    ),
                    "overlap_ratio_median_delta_b_minus_a": (
                        right["overlap_ratio_median"] - left["overlap_ratio_median"]
                    ),
                    "span_ranges_overlap": (
                        max(left["span_min_ms"], right["span_min_ms"])
                        <= min(left["span_max_ms"], right["span_max_ms"])
                    ),
                    "operation_ranges_overlap": (
                        max(left["operation_min_ms"], right["operation_min_ms"])
                        <= min(left["operation_max_ms"], right["operation_max_ms"])
                    ),
                }
            )

    _write_csv(
        input_dir / "measurements.csv",
        ("streams", "run", "sqlite", "span_ns", "span_ms"),
        [
            {
                key: profile[key]
                for key in ("streams", "run", "sqlite", "span_ns", "span_ms")
            }
            for profile in profiles
        ],
    )
    _write_csv(
        input_dir / "summary.csv",
        (
            "streams",
            "runs",
            "operations",
            "span_median_ms",
            "span_min_ms",
            "span_max_ms",
            "operation_mean_ms",
            "operation_median_ms",
            "operation_min_ms",
            "operation_max_ms",
            "duration_sum_median_ms",
            "duration_sum_min_ms",
            "duration_sum_max_ms",
            "overlap_ratio_median",
            "overlap_ratio_min",
            "overlap_ratio_max",
        ),
        summaries,
    )
    _write_csv(
        input_dir / "comparison.csv",
        (
            "streams_a",
            "streams_b",
            "span_median_delta_ms_b_minus_a",
            "operation_median_delta_ms_b_minus_a",
            "operation_median_delta_percent_vs_a",
            "overlap_ratio_median_delta_b_minus_a",
            "span_ranges_overlap",
            "operation_ranges_overlap",
        ),
        comparisons,
    )
    _write_csv(
        input_dir / "run_metrics.csv",
        (
            "streams",
            "run",
            "sqlite",
            "span_ns",
            "span_ms",
            "duration_sum_ns",
            "duration_sum_ms",
            "mean_duration_ms",
            "overlap_ratio",
        ),
        run_metrics,
    )
    _write_csv(
        input_dir / "memsets.csv",
        (
            "streams",
            "run",
            "sqlite",
            "task",
            "stream_id",
            "start_ms",
            "duration_ns",
            "duration_ms",
        ),
        memsets,
    )

    for profile in profiles:
        print(
            f"\n{profile['streams']} Streams run {profile['run']:02d} "
            f"({profile['sqlite']})"
        )
        print("Task  Stream  Start(ms)  Duration(ms)")
        for memset in profile["memsets"]:
            print(
                f"{memset['task']:<5} "
                f"{memset['stream_id']:<7} "
                f"{memset['start_ms']:>9.4f} "
                f"{memset['duration_ms']:>12.4f}"
            )
        print(
            f"Timeline span: {profile['span_ms']:.4f} ms; "
            f"Memset duration sum: {profile['duration_sum_ms']:.4f} ms; "
            f"mean per Memset: {profile['mean_duration_ms']:.4f} ms; "
            f"overlap ratio R: {profile['overlap_ratio']:.4f}"
        )
    print("\nSummary:")
    for summary in summaries:
        print(
            f"{summary['streams']} Streams ({summary['runs']} runs, "
            f"{summary['operations']} Memsets): "
            f"timeline median/min/max "
            f"{summary['span_median_ms']:.4f}/{summary['span_min_ms']:.4f}/"
            f"{summary['span_max_ms']:.4f} ms; per-Memset mean/median/min/max "
            f"{summary['operation_mean_ms']:.4f}/{summary['operation_median_ms']:.4f}/"
            f"{summary['operation_min_ms']:.4f}/{summary['operation_max_ms']:.4f} ms; "
            f"duration-sum median {summary['duration_sum_median_ms']:.4f} ms; "
            f"R median/min/max {summary['overlap_ratio_median']:.4f}/"
            f"{summary['overlap_ratio_min']:.4f}/{summary['overlap_ratio_max']:.4f}"
        )
    print("\nMedian distribution comparison:")
    for comparison in comparisons:
        delta = comparison["operation_median_delta_ms_b_minus_a"]
        percent = comparison["operation_median_delta_percent_vs_a"]
        percent_text = (
            f"{percent:+.2f}%" if percent != "" else "undefined (zero baseline)"
        )
        overlap = "yes" if comparison["operation_ranges_overlap"] else "no"
        print(
            f"{comparison['streams_a']} vs {comparison['streams_b']} Streams: "
            f"per-Memset median delta {delta:+.4f} ms ({percent_text}); "
            f"R median delta "
            f"{comparison['overlap_ratio_median_delta_b_minus_a']:+.4f}; "
            f"per-Memset min-max ranges overlap: {overlap}"
        )
    print(f"\nCSV reports written to {input_dir}")
    return profiles, summaries, comparisons


def _write_csv(path, fieldnames, rows):
    with path.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(
        description="Summarize scheduler Nsight Systems SQLite exports."
    )
    parser.add_argument(
        "input_dir",
        nargs="?",
        type=Path,
        default=Path("."),
        help="directory containing scheduler SQLite exports (default: current directory)",
    )
    args = parser.parse_args()
    analyze_profiles(args.input_dir)


if __name__ == "__main__":
    main()
