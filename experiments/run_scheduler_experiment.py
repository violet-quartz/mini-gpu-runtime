import argparse
from datetime import datetime
import shutil
import subprocess
from pathlib import Path

from analyze_scheduler import analyze_profiles


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
STREAM_COUNTS = (1, 3, 6)
REPEATS = 10


def main():
    parser = argparse.ArgumentParser(
        description="Profile the scheduler 10 times for each stream count (1, 3, 6)."
    )
    parser.add_argument(
        "--executable",
        type=Path,
        default=REPO_ROOT / "build" / "profile_scheduler",
        help="profile_scheduler executable",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        help="output directory (defaults to a timestamped directory under experiments/results)",
    )
    args = parser.parse_args()

    executable = args.executable.resolve()
    if not executable.is_file():
        parser.error(f"profile executable does not exist: {executable}")
    if not executable.stat().st_mode & 0o111:
        parser.error(f"profile executable is not executable: {executable}")

    nsys = shutil.which("nsys")
    if nsys is None:
        parser.error("Nsight Systems CLI 'nsys' was not found on PATH")

    if args.output_dir is None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        output_dir = SCRIPT_DIR / "results" / f"run_{timestamp}"
    else:
        output_dir = args.output_dir.resolve()
    try:
        output_dir.mkdir(parents=True, exist_ok=False)
    except FileExistsError:
        parser.error(f"output directory already exists; refusing to overwrite: {output_dir}")

    for streams in STREAM_COUNTS:
        for run in range(1, REPEATS + 1):
            name = f"streams_{streams}_run_{run:02d}"
            report = output_dir / f"{name}.nsys-rep"
            sqlite_export = output_dir / f"{name}.sqlite"
            print(
                f"Profiling {streams} streams, run {run}/{REPEATS}",
                flush=True,
            )
            subprocess.run(
                [
                    nsys,
                    "profile",
                    "--trace=cuda",
                    "--sample=none",
                    "--stats=false",
                    f"--output={output_dir / name}",
                    str(executable),
                    str(streams),
                ],
                check=True,
            )
            if not report.is_file():
                raise FileNotFoundError(f"Nsight Systems did not create {report}")

            subprocess.run(
                [
                    nsys,
                    "export",
                    "--type=sqlite",
                    f"--output={sqlite_export}",
                    str(report),
                ],
                check=True,
            )
            if not sqlite_export.is_file():
                raise FileNotFoundError(f"Nsight Systems did not create {sqlite_export}")

    analyze_profiles(output_dir)


if __name__ == "__main__":
    main()
