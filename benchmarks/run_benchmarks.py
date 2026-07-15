#!/usr/bin/env python3
"""Run reproducible Planar benchmark profiles into immutable schema-v1 bundles."""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import hashlib
import json
import math
import os
import platform
import re
import shutil
import signal
import statistics
import subprocess
import sys
import time
from collections import Counter
from pathlib import Path
from typing import Any

from render_chart import render as render_chart, render_distributions


SCHEMA_VERSION = 1
STATUS_VALUES = {"ok", "timeout", "oom", "error", "skipped", "validation_failed"}
TIMING_FIELDS = (
    "input_generation_seconds",
    "triangulation_seconds",
    "voronoi_seconds",
    "validation_seconds",
    "compute_total_seconds",
)


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def logical_path(path: Path, repo: Path, external_label: str) -> str:
    """Return a stable publication-safe path without recording the workstation root."""
    resolved = path.resolve()
    try:
        return resolved.relative_to(repo.resolve()).as_posix()
    except ValueError:
        return f"${external_label}/{resolved.name}"


def json_bytes(value: Any) -> bytes:
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode("utf-8")


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(json_bytes(value))


def command_output(command: list[str], cwd: Path | None = None) -> str | None:
    try:
        return subprocess.check_output(
            command,
            cwd=cwd,
            text=True,
            stderr=subprocess.STDOUT,
            timeout=10,
        ).strip()
    except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return None


def compiler_version_line(compiler_command: str) -> str | None:
    output = command_output([compiler_command, "--version"])
    if not output:
        return None
    lines = output.splitlines()
    return lines[0].strip() if lines else None


def git_metadata(repo: Path) -> dict[str, Any]:
    commit = command_output(["git", "rev-parse", "HEAD"], repo) or "unknown"
    branch = command_output(["git", "rev-parse", "--abbrev-ref", "HEAD"], repo) or "unknown"
    status = command_output(["git", "status", "--porcelain=v1", "--untracked-files=all"], repo) or ""
    diff = command_output(["git", "diff", "--binary", "HEAD"], repo) or ""
    return {
        "commit": commit,
        "branch": branch,
        "dirty": bool(status),
        "status_porcelain": status.splitlines(),
        "working_diff_sha256": sha256_bytes(diff.encode("utf-8")) if diff else None,
    }


def memory_total_kb() -> int | None:
    try:
        for line in Path("/proc/meminfo").read_text(encoding="utf-8").splitlines():
            if line.startswith("MemTotal:"):
                return int(line.split()[1])
    except (OSError, ValueError):
        pass
    return None


def cpu_model() -> str | None:
    try:
        for line in Path("/proc/cpuinfo").read_text(encoding="utf-8").splitlines():
            if line.lower().startswith("model name"):
                return line.split(":", 1)[1].strip()
    except OSError:
        pass
    return None


def collect_environment(repo: Path, compiler_command: str) -> dict[str, Any]:
    compiler = command_output([compiler_command, "--version"])
    gnu_time = command_output(["/usr/bin/time", "--version"])
    return {
        "schema_version": SCHEMA_VERSION,
        "captured_at_utc": utc_now(),
        "platform": {
            "system": platform.system(),
            "release": platform.release(),
            "version": platform.version(),
            "machine": platform.machine(),
            "python": platform.python_version(),
            "wsl_detected": (
                "microsoft" in platform.release().lower()
                or "microsoft" in platform.version().lower()
            ),
        },
        "hardware": {
            "cpu_model": cpu_model(),
            "logical_cpu_count": os.cpu_count(),
            "memory_total_kb": memory_total_kb(),
            "lscpu": command_output(["lscpu"]),
        },
        "tools": {
            "compiler_command": compiler_command,
            "compiler": compiler,
            "gnu_time": gnu_time,
        },
        "environment_variables": {
            key: os.environ.get(key)
            for key in ("LANG", "LC_ALL", "TZ", "OMP_NUM_THREADS")
        },
        "repository_path": "$REPO_ROOT",
        "path_capture_policy": "PATH and absolute workstation paths are intentionally not recorded",
    }


def inspect_driver(binary: Path, repo: Path, seed: int) -> dict[str, Any]:
    command = [
        str(binary), "--count=10", f"--seed={seed}", "--coord-max=1000000",
        "--distribution=uniform",
    ]
    try:
        completed = subprocess.run(
            command,
            cwd=repo,
            text=True,
            capture_output=True,
            timeout=15,
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise ValueError(f"benchmark binary preflight failed: {exc}") from exc
    if completed.returncode != 0:
        raise ValueError(
            f"benchmark binary preflight exited {completed.returncode}: {completed.stderr.strip()}"
        )
    try:
        payload = json.loads(completed.stdout)
    except json.JSONDecodeError as exc:
        raise ValueError("benchmark binary preflight did not emit valid JSON") from exc
    return payload.get("build", {})


def parse_time_file(path: Path) -> dict[str, float | int | None]:
    values: dict[str, float | int | None] = {
        "elapsed_seconds": None,
        "user_seconds": None,
        "system_seconds": None,
        "max_rss_kb": None,
    }
    if not path.exists():
        return values
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "=" not in line:
            continue
        key, raw = line.split("=", 1)
        if key not in values:
            continue
        try:
            values[key] = int(raw) if key == "max_rss_kb" else float(raw)
        except ValueError:
            values[key] = None
    return values


def run_with_timeout(
    command: list[str], cwd: Path, timeout_seconds: float, stdout_path: Path,
    stderr_path: Path, time_path: Path,
) -> tuple[int | None, bool, float]:
    measured_command = [
        "/usr/bin/time", "-f",
        "elapsed_seconds=%e\nuser_seconds=%U\nsystem_seconds=%S\nmax_rss_kb=%M\nexit_code=%x",
        "-o", str(time_path), "--", *command,
    ]
    environment = os.environ.copy()
    environment["LC_ALL"] = "C"
    started = time.perf_counter()
    process = subprocess.Popen(
        measured_command,
        cwd=cwd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        start_new_session=True,
        env=environment,
    )
    timed_out = False
    try:
        stdout, stderr = process.communicate(timeout=timeout_seconds)
    except subprocess.TimeoutExpired:
        timed_out = True
        os.killpg(process.pid, signal.SIGTERM)
        try:
            stdout, stderr = process.communicate(timeout=2)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            stdout, stderr = process.communicate()
    external_wall = time.perf_counter() - started
    stdout_path.write_text(stdout, encoding="utf-8")
    stderr_path.write_text(stderr, encoding="utf-8")
    if not time_path.exists():
        time_path.write_text("", encoding="utf-8")
    return (None if timed_out else process.returncode), timed_out, external_wall


def percentile(values: list[float], probability: float) -> float | None:
    if not values:
        return None
    ordered = sorted(values)
    position = (len(ordered) - 1) * probability
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return ordered[lower]
    return ordered[lower] + (ordered[upper] - ordered[lower]) * (position - lower)


def robust_statistics(values: list[float]) -> dict[str, float | int | None]:
    if not values:
        return {key: None for key in ("count", "median", "mad", "p10", "p90", "minimum", "maximum")}
    median = statistics.median(values)
    return {
        "count": len(values),
        "median": median,
        "mad": statistics.median(abs(value - median) for value in values),
        "p10": percentile(values, 0.10),
        "p90": percentile(values, 0.90),
        "minimum": min(values),
        "maximum": max(values),
    }


def create_summary(run_id: str, profile_name: str, profile: dict[str, Any], records: list[dict[str, Any]]) -> dict[str, Any]:
    cases = []
    for case in profile["cases"]:
        case_records = [record for record in records if record["case_id"] == case["id"]]
        successful = [record for record in case_records if record["status"] == "ok"]
        statistics_by_field = {
            field: robust_statistics([
                float(record["measurements"][field])
                for record in successful
                if record["measurements"][field] is not None
            ])
            for field in TIMING_FIELDS
        }
        statistics_by_field["external_wall_seconds"] = robust_statistics([
            float(record["measurements"]["external_wall_seconds"])
            for record in successful
            if record["measurements"]["external_wall_seconds"] is not None
        ])
        statistics_by_field["max_rss_kb"] = robust_statistics([
            float(record["measurements"]["max_rss_kb"])
            for record in successful
            if record["measurements"]["max_rss_kb"] is not None
        ])
        cases.append({
            "case_id": case["id"],
            "input": case,
            "planned_repetitions": profile["repetitions"],
            "successful_repetitions": len(successful),
            "status_counts": dict(sorted(Counter(record["status"] for record in case_records).items())),
            "statistics": statistics_by_field,
        })
    status_counts = dict(sorted(Counter(record["status"] for record in records).items()))
    distribution_aggregates = []
    if profile.get("aggregate_by_distribution"):
        distributions = list(dict.fromkeys(case["distribution"] for case in profile["cases"]))
        for distribution in distributions:
            distribution_cases = [
                case for case in cases if case["input"]["distribution"] == distribution
            ]
            case_ids = {case["case_id"] for case in distribution_cases}
            distribution_records = [
                record for record in records if record["case_id"] in case_ids
            ]
            successful = [
                record for record in distribution_records if record["status"] == "ok"
            ]
            seed_case_medians = [
                float(case["statistics"]["compute_total_seconds"]["median"])
                for case in distribution_cases
                if case["statistics"]["compute_total_seconds"]["median"] is not None
            ]
            pooled_measurements = [
                float(record["measurements"]["compute_total_seconds"])
                for record in successful
                if record["measurements"]["compute_total_seconds"] is not None
            ]
            distribution_aggregates.append({
                "distribution": distribution,
                "case_count": len(distribution_cases),
                "distinct_seed_count": len({
                    int(case["input"]["seed"]) for case in distribution_cases
                }),
                "planned_run_count": len(distribution_cases) * int(profile["repetitions"]),
                "successful_run_count": len(successful),
                "status_counts": dict(sorted(Counter(
                    record["status"] for record in distribution_records
                ).items())),
                "compute_total_seconds_across_seed_case_medians":
                    robust_statistics(seed_case_medians),
                "compute_total_seconds_across_all_repetitions":
                    robust_statistics(pooled_measurements),
            })
    return {
        "schema_version": SCHEMA_VERSION,
        "run_id": run_id,
        "profile": profile_name,
        "statistic_policy": "successful runs only; median, median absolute deviation, linearly interpolated p10/p90, min/max",
        "sample_accounting": {
            "case_count": len(profile["cases"]),
            "planned_run_count": len(profile["cases"]) * int(profile["repetitions"]),
            "observed_run_count": len(records),
            "successful_run_count": status_counts.get("ok", 0),
            "status_counts": status_counts,
        },
        "distribution_aggregate_policy": (
            "For each distribution, seed-level uncertainty summarizes the case median "
            "for each distinct deterministic input. The pooled repetition summary is "
            "reported separately because repeated timings are not independent inputs."
        ),
        "distribution_aggregates": distribution_aggregates,
        "cases": cases,
    }


def write_summary_csv(path: Path, summary: dict[str, Any]) -> None:
    fields = [
        "case_id", "distribution", "requested_count", "seed",
        "planned_repetitions", "successful_repetitions",
        "ok", "timeout", "error", "validation_failed", "compute_median_seconds",
        "compute_mad_seconds", "compute_p10_seconds", "compute_p90_seconds",
        "external_wall_median_seconds", "max_rss_median_kb",
    ]
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, lineterminator="\n")
        writer.writeheader()
        for case in summary["cases"]:
            statuses = case["status_counts"]
            compute = case["statistics"]["compute_total_seconds"]
            writer.writerow({
                "case_id": case["case_id"],
                "distribution": case["input"]["distribution"],
                "requested_count": case["input"]["count"],
                "seed": case["input"]["seed"],
                "planned_repetitions": case["planned_repetitions"],
                "successful_repetitions": case["successful_repetitions"],
                "ok": statuses.get("ok", 0),
                "timeout": statuses.get("timeout", 0),
                "error": statuses.get("error", 0),
                "validation_failed": statuses.get("validation_failed", 0),
                "compute_median_seconds": compute["median"],
                "compute_mad_seconds": compute["mad"],
                "compute_p10_seconds": compute["p10"],
                "compute_p90_seconds": compute["p90"],
                "external_wall_median_seconds": case["statistics"]["external_wall_seconds"]["median"],
                "max_rss_median_kb": case["statistics"]["max_rss_kb"]["median"],
            })


def write_distribution_summary_csv(path: Path, summary: dict[str, Any]) -> None:
    fields = [
        "distribution", "case_count", "distinct_seed_count", "planned_run_count",
        "successful_run_count", "ok", "timeout", "error", "validation_failed",
        "seed_case_median_sample_count", "seed_case_compute_median_seconds",
        "seed_case_compute_mad_seconds", "seed_case_compute_p10_seconds",
        "seed_case_compute_p90_seconds", "pooled_repetition_sample_count",
        "pooled_compute_median_seconds", "pooled_compute_mad_seconds",
        "pooled_compute_p10_seconds", "pooled_compute_p90_seconds",
    ]
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, lineterminator="\n")
        writer.writeheader()
        for aggregate in summary["distribution_aggregates"]:
            statuses = aggregate["status_counts"]
            seed_cases = aggregate["compute_total_seconds_across_seed_case_medians"]
            pooled = aggregate["compute_total_seconds_across_all_repetitions"]
            writer.writerow({
                "distribution": aggregate["distribution"],
                "case_count": aggregate["case_count"],
                "distinct_seed_count": aggregate["distinct_seed_count"],
                "planned_run_count": aggregate["planned_run_count"],
                "successful_run_count": aggregate["successful_run_count"],
                "ok": statuses.get("ok", 0),
                "timeout": statuses.get("timeout", 0),
                "error": statuses.get("error", 0),
                "validation_failed": statuses.get("validation_failed", 0),
                "seed_case_median_sample_count": seed_cases["count"],
                "seed_case_compute_median_seconds": seed_cases["median"],
                "seed_case_compute_mad_seconds": seed_cases["mad"],
                "seed_case_compute_p10_seconds": seed_cases["p10"],
                "seed_case_compute_p90_seconds": seed_cases["p90"],
                "pooled_repetition_sample_count": pooled["count"],
                "pooled_compute_median_seconds": pooled["median"],
                "pooled_compute_mad_seconds": pooled["mad"],
                "pooled_compute_p10_seconds": pooled["p10"],
                "pooled_compute_p90_seconds": pooled["p90"],
            })


def write_checksums(bundle: Path) -> None:
    lines = []
    for path in sorted(item for item in bundle.rglob("*") if item.is_file() and item.name != "checksums.sha256"):
        lines.append(f"{sha256_file(path)}  {path.relative_to(bundle).as_posix()}")
    (bundle / "checksums.sha256").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", choices=("smoke", "standard", "headline", "research"), default="smoke")
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--compiler-flags", required=True)
    parser.add_argument("--expected-build-profile", default="development-normal")
    parser.add_argument("--output-root", type=Path, default=Path("benchmarks/runs"))
    parser.add_argument("--run-id")
    parser.add_argument("--allow-dirty", action="store_true")
    args = parser.parse_args()

    benchmark_dir = Path(__file__).resolve().parent
    repo = benchmark_dir.parent
    profiles_document = json.loads((benchmark_dir / "profiles.json").read_text(encoding="utf-8"))
    profile = profiles_document["profiles"][args.profile]
    binary = args.binary if args.binary.is_absolute() else (repo / args.binary)
    binary = binary.resolve()
    if not binary.is_file():
        parser.error(f"benchmark binary does not exist: {binary}")

    git = git_metadata(repo)
    if git["dirty"] and not args.allow_dirty:
        parser.error("refusing to benchmark a dirty repository; use --allow-dirty only for scratch runs")
    resolved_compiler_version = compiler_version_line(args.compiler)
    if not resolved_compiler_version:
        parser.error(f"cannot execute compiler version probe: {args.compiler} --version")
    try:
        driver_build = inspect_driver(binary, repo, int(profile["cases"][0]["seed"]))
    except ValueError as exc:
        parser.error(str(exc))
    expected_build = {
        "commit": git["commit"],
        "dirty": git["dirty"],
        "profile": args.expected_build_profile,
        "compiler_command": args.compiler,
        "compiler_version": resolved_compiler_version,
        "compiler_flags": args.compiler_flags,
    }
    mismatches = {
        key: {"expected": value, "actual": driver_build.get(key)}
        for key, value in expected_build.items()
        if driver_build.get(key) != value
    }
    if mismatches:
        parser.error(
            "benchmark binary provenance is stale or incoherent; rebuild it from this tree: "
            + json.dumps(mismatches, sort_keys=True)
        )

    binary_sha256 = sha256_file(binary)
    binary_logical_path = logical_path(binary, repo, "EXTERNAL_BINARY")
    default_run_id = f"{utc_now().replace(':', '').replace('-', '')}-{git['commit'][:8]}-{args.profile}"
    run_id = args.run_id or default_run_id
    if not re.fullmatch(r"[A-Za-z0-9._-]+", run_id):
        parser.error("--run-id may contain only letters, digits, dot, underscore, and hyphen")

    output_root = args.output_root if args.output_root.is_absolute() else repo / args.output_root
    final_bundle = output_root / run_id
    partial_bundle = output_root / f".{run_id}.partial-{os.getpid()}"
    if final_bundle.exists() or partial_bundle.exists():
        parser.error(f"refusing to overwrite existing bundle or partial directory for {run_id}")

    output_root.mkdir(parents=True, exist_ok=True)
    partial_bundle.mkdir()
    (partial_bundle / "logs").mkdir()
    (partial_bundle / "inputs").mkdir()
    (partial_bundle / "charts").mkdir()
    started_at = utc_now()
    records: list[dict[str, Any]] = []

    try:
        environment = collect_environment(repo, args.compiler)
        write_json(partial_bundle / "environment.json", environment)

        for case in profile["cases"]:
            for repetition in range(1, int(profile["repetitions"]) + 1):
                stem = f"{case['id']}-r{repetition:03d}"
                stdout_path = partial_bundle / "logs" / f"{stem}.stdout.json"
                stderr_path = partial_bundle / "logs" / f"{stem}.stderr.txt"
                time_path = partial_bundle / "logs" / f"{stem}.time.txt"
                command = [
                    str(binary),
                    f"--count={case['count']}",
                    f"--seed={case['seed']}",
                    f"--coord-max={case['coord_max']}",
                    f"--distribution={case['distribution']}",
                    "--emit-points",
                ]
                recorded_command = [binary_logical_path, *command[1:]]
                exit_code, timed_out, external_wall = run_with_timeout(
                    command, repo, float(profile["timeout_seconds"]),
                    stdout_path, stderr_path, time_path,
                )
                time_values = parse_time_file(time_path)
                driver: dict[str, Any] | None = None
                try:
                    driver = json.loads(stdout_path.read_text(encoding="utf-8"))
                except (json.JSONDecodeError, OSError):
                    driver = None

                status = "timeout" if timed_out else "error"
                if driver is not None and not timed_out:
                    driver_status = driver.get("status")
                    if exit_code == 0 and driver_status == "ok":
                        status = "ok"
                    elif driver_status == "validation_failed":
                        status = "validation_failed"

                timing = driver.get("timing_seconds", {}) if driver else {}
                driver_input = driver.get("input", {}) if driver else {}
                points = driver_input.pop("points", None)
                input_sha256 = None
                input_path = None
                if points is not None:
                    exact_input = {
                        "schema_version": SCHEMA_VERSION,
                        "generator": driver_input.get("generator"),
                        "distribution": driver_input.get("distribution"),
                        "seed": driver_input.get("seed"),
                        "coord_max": driver_input.get("coord_max"),
                        "points": points,
                    }
                    payload = json_bytes(exact_input)
                    input_sha256 = sha256_bytes(payload)
                    input_path = partial_bundle / "inputs" / f"{input_sha256}.json"
                    if not input_path.exists():
                        input_path.write_bytes(payload)

                record = {
                    "schema_version": SCHEMA_VERSION,
                    "run_id": run_id,
                    "case_id": case["id"],
                    "repetition": repetition,
                    "status": status,
                    "command": recorded_command,
                    "timeout_seconds": float(profile["timeout_seconds"]),
                    "exit_code": exit_code,
                    "measurements": {
                        "external_wall_seconds": external_wall,
                        "user_seconds": time_values["user_seconds"],
                        "system_seconds": time_values["system_seconds"],
                        "max_rss_kb": time_values["max_rss_kb"],
                        "input_generation_seconds": timing.get("input_generation"),
                        "triangulation_seconds": timing.get("triangulation"),
                        "voronoi_seconds": timing.get("voronoi"),
                        "validation_seconds": timing.get("validation"),
                        "compute_total_seconds": timing.get("compute_total"),
                    },
                    "input": {
                        **driver_input,
                        "points_sha256": input_sha256,
                        "points_artifact": input_path.relative_to(partial_bundle).as_posix() if input_path else None,
                    },
                    "output": driver.get("topology", {}) if driver else {},
                    "validation": driver.get("validation", {}) if driver else {},
                    "driver_build": driver.get("build", {}) if driver else {},
                    "raw_artifacts": {
                        "stdout": stdout_path.relative_to(partial_bundle).as_posix(),
                        "stderr": stderr_path.relative_to(partial_bundle).as_posix(),
                        "resource_time": time_path.relative_to(partial_bundle).as_posix(),
                    },
                }
                if status not in STATUS_VALUES:
                    raise RuntimeError(f"invalid status {status}")
                records.append(record)

        with (partial_bundle / "runs.jsonl").open("w", encoding="utf-8") as handle:
            for record in records:
                handle.write(json.dumps(record, sort_keys=True, separators=(",", ":")) + "\n")

        summary = create_summary(run_id, args.profile, profile, records)
        write_json(partial_bundle / "summary.json", summary)
        write_summary_csv(partial_bundle / "summary.csv", summary)
        write_distribution_summary_csv(partial_bundle / "distribution-summary.csv", summary)
        render_chart(partial_bundle / "summary.json", partial_bundle / "charts" / "phase-medians.svg")
        render_distributions(partial_bundle / "summary.json", partial_bundle / "charts" / "distribution-comparison.svg")

        case_validation = {}
        for case in profile["cases"]:
            case_records = [record for record in records if record["case_id"] == case["id"]]
            input_hashes = {record["input"].get("points_sha256") for record in case_records if record["input"].get("points_sha256")}
            output_hashes = {record["validation"].get("canonical_output_hash") for record in case_records if record["validation"].get("canonical_output_hash")}
            case_validation[case["id"]] = {
                "all_runs_ok": all(record["status"] == "ok" for record in case_records),
                "input_stable_across_repetitions": len(input_hashes) == 1,
                "output_stable_across_repetitions": len(output_hashes) == 1,
                "input_sha256": next(iter(input_hashes)) if len(input_hashes) == 1 else None,
                "canonical_output_hash": next(iter(output_hashes)) if len(output_hashes) == 1 else None,
            }
        all_runs_ok = all(record["status"] == "ok" for record in records)
        validation = {
            "schema_version": SCHEMA_VERSION,
            "all_runs_ok": all_runs_ok,
            "all_inputs_stable": all(value["input_stable_across_repetitions"] for value in case_validation.values()),
            "all_outputs_stable": all(value["output_stable_across_repetitions"] for value in case_validation.values()),
            "provenance_preflight_passed": True,
            "cases": case_validation,
            "known_unrelated_test_failures_masked": False,
        }
        write_json(partial_bundle / "validation.json", validation)

        status_counts = dict(sorted(Counter(record["status"] for record in records).items()))
        if sha256_file(binary) != binary_sha256:
            raise RuntimeError("benchmark binary changed after provenance preflight")
        manifest = {
            "schema_version": SCHEMA_VERSION,
            "bundle_kind": "planar_benchmark_run_bundle",
            "run_id": run_id,
            "profile": args.profile,
            "profile_definition": profile,
            "started_at_utc": started_at,
            "finished_at_utc": utc_now(),
            "repository": git,
            "binary": {"path": binary_logical_path, "sha256": binary_sha256},
            "build_contract": {
                "compiler": args.compiler,
                "compiler_flags": args.compiler_flags,
                "contract_version": 2,
                "compiler_command": args.compiler,
                "compiler_version": resolved_compiler_version,
                "expected_driver_profile": args.expected_build_profile,
                "preflight_driver_build": driver_build,
                "provenance_preflight_passed": True,
            },
            "runner_command": [
                "python3", "benchmarks/run_benchmarks.py",
                f"--profile={args.profile}",
                f"--binary={binary_logical_path}",
                f"--compiler={args.compiler}",
                f"--compiler-flags={args.compiler_flags}",
                f"--expected-build-profile={args.expected_build_profile}",
                f"--output-root={logical_path(output_root, repo, 'OUTPUT_ROOT')}",
                f"--run-id={run_id}",
                *(["--allow-dirty"] if args.allow_dirty else []),
            ],
            "working_directory": "$REPO_ROOT",
            "run_count": len(records),
            "status_counts": status_counts,
            "metric_definition": {
                "compute_total_seconds": "steady-clock triangulation + Voronoi construction + Delaunay/topology validation; excludes deterministic input generation and serialization",
                "external_wall_seconds": "Python monotonic wall time around GNU time and the benchmark process",
                "max_rss_kb": "GNU time %M peak resident set size for the benchmark process",
            },
            "artifacts": {
                "records": "runs.jsonl",
                "summary_json": "summary.json",
                "summary_csv": "summary.csv",
                "distribution_summary_csv": "distribution-summary.csv",
                "distribution_chart": "charts/distribution-comparison.svg",
                "environment": "environment.json",
                "validation": "validation.json",
                "chart": "charts/phase-medians.svg",
                "checksums": "checksums.sha256",
            },
        }
        write_json(partial_bundle / "manifest.json", manifest)
        write_checksums(partial_bundle)
        partial_bundle.rename(final_bundle)
    except BaseException:
        if partial_bundle.exists():
            shutil.rmtree(partial_bundle)
        raise

    validator = benchmark_dir / "validate_bundle.py"
    validation_process = subprocess.run([sys.executable, str(validator), str(final_bundle)], cwd=repo)
    print(final_bundle)
    if validation_process.returncode != 0:
        return validation_process.returncode
    return 0 if all(record["status"] == "ok" for record in records) else 1


if __name__ == "__main__":
    raise SystemExit(main())
