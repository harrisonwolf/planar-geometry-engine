#!/usr/bin/env python3
"""Regression tests for benchmark provenance coherence."""

from __future__ import annotations

import hashlib
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


BENCHMARK_DIR = Path(__file__).resolve().parent
REPO = BENCHMARK_DIR.parent
sys.path.insert(0, str(BENCHMARK_DIR))

from run_benchmarks import (  # noqa: E402
    create_summary,
    write_distribution_summary_csv,
    write_summary_csv,
)
from validate_bundle import validate  # noqa: E402


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


CONTRACT_COMPILER_COMMAND = "g++"
CONTRACT_COMPILER_VERSION = "g++ test compiler 1.0"
CONTRACT_COMPILER_FLAGS = "-Wall -Wextra -std=c++17 -O2"


def make_bundle(
    root: Path, *, driver_commit: str, driver_profile: str,
    record_distribution: str = "uniform",
    artifact_distribution: str | None = None,
    driver_compiler_command: str = CONTRACT_COMPILER_COMMAND,
    driver_compiler_version: str = CONTRACT_COMPILER_VERSION,
    driver_compiler_flags: str = CONTRACT_COMPILER_FLAGS,
) -> Path:
    bundle = root / "bundle"
    (bundle / "logs").mkdir(parents=True)
    (bundle / "charts").mkdir()
    (bundle / "inputs").mkdir()
    artifact_path = bundle / "inputs" / "exact.json"
    write_json(artifact_path, {
        "schema_version": 1,
        "generator": "test_generator_v1",
        "distribution": artifact_distribution or record_distribution,
        "seed": 7,
        "coord_max": 100,
        "points": [[index, index] for index in range(10)],
    })
    commit = "a" * 40
    binary_path = "build/development/normal/benchmark_driver"
    manifest = {
        "schema_version": 1,
        "bundle_kind": "planar_benchmark_run_bundle",
        "run_id": "test",
        "profile": "smoke",
        "repository": {"commit": commit, "dirty": False},
        "binary": {"path": binary_path, "sha256": "c" * 64},
        "working_directory": "$REPO_ROOT",
        "runner_command": ["python3", "benchmarks/run_benchmarks.py", "--profile=smoke"],
        "profile_definition": {
            "repetitions": 1,
            "aggregate_by_distribution": True,
            "cases": [{
                "id": "n10", "distribution": "uniform", "count": 10,
                "seed": 7, "coord_max": 100,
            }],
        },
        "build_contract": {
            "contract_version": 2,
            "compiler": CONTRACT_COMPILER_COMMAND,
            "compiler_command": CONTRACT_COMPILER_COMMAND,
            "compiler_version": CONTRACT_COMPILER_VERSION,
            "compiler_flags": CONTRACT_COMPILER_FLAGS,
            "expected_driver_profile": "development-normal",
            "preflight_driver_build": {
                "commit": commit,
                "dirty": False,
                "profile": "development-normal",
                "compiler_command": CONTRACT_COMPILER_COMMAND,
                "compiler_version": CONTRACT_COMPILER_VERSION,
                "compiler_flags": CONTRACT_COMPILER_FLAGS,
            },
            "provenance_preflight_passed": True,
        },
        "run_count": 1,
        "artifacts": {
            "distribution_summary_csv": "distribution-summary.csv",
        },
    }
    record = {
        "schema_version": 1,
        "run_id": "test",
        "case_id": "n10",
        "repetition": 1,
        "status": "ok",
        "command": [binary_path, "--count=10", f"--distribution={record_distribution}"],
        "timeout_seconds": 10.0,
        "exit_code": 0,
        "driver_build": {
            "commit": driver_commit,
            "dirty": False,
            "profile": driver_profile,
            "compiler_command": driver_compiler_command,
            "compiler_version": driver_compiler_version,
            "compiler_flags": driver_compiler_flags,
        },
        "measurements": {
            "external_wall_seconds": 0.1,
            "user_seconds": 0.0,
            "system_seconds": 0.0,
            "max_rss_kb": 1000,
            "input_generation_seconds": 0.0,
            "triangulation_seconds": 0.01,
            "voronoi_seconds": 0.01,
            "validation_seconds": 0.01,
            "compute_total_seconds": 0.03,
        },
        "input": {
            "distribution": record_distribution,
            "seed": 7,
            "requested_count": 10,
            "coord_max": 100,
            "points_artifact": "inputs/exact.json",
            "points_sha256": sha256(artifact_path),
        },
        "output": {},
        "validation": {},
        "raw_artifacts": {
            "stdout": "logs/run.stdout.json",
            "stderr": "logs/run.stderr.txt",
            "resource_time": "logs/run.time.txt",
        },
    }
    write_json(bundle / "manifest.json", manifest)
    write_json(bundle / "environment.json", {
        "repository_path": "$REPO_ROOT",
        "environment_variables": {},
    })
    (bundle / "runs.jsonl").write_text(json.dumps(record, sort_keys=True) + "\n", encoding="utf-8")
    summary = create_summary(
        "test", "smoke", manifest["profile_definition"], [record]
    )
    write_json(bundle / "summary.json", summary)
    write_summary_csv(bundle / "summary.csv", summary)
    write_distribution_summary_csv(bundle / "distribution-summary.csv", summary)
    write_json(bundle / "validation.json", {})
    (bundle / "charts" / "phase-medians.svg").write_text("<svg/>\n", encoding="utf-8")
    (bundle / "logs" / "run.stdout.json").write_text("{}\n", encoding="utf-8")
    (bundle / "logs" / "run.stderr.txt").write_text("", encoding="utf-8")
    (bundle / "logs" / "run.time.txt").write_text("", encoding="utf-8")
    files = sorted(path for path in bundle.rglob("*") if path.is_file())
    (bundle / "checksums.sha256").write_text(
        "".join(f"{sha256(path)}  {path.relative_to(bundle).as_posix()}\n" for path in files),
        encoding="utf-8",
    )
    return bundle


class BundleProvenanceTests(unittest.TestCase):
    def test_validator_accepts_publication_safe_logical_paths(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            self.assertEqual(validate(bundle), [])

    def test_validator_rejects_absolute_manifest_paths(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            manifest_path = bundle / "manifest.json"
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
            manifest["binary"]["path"] = "/home/example/private/benchmark_driver"
            write_json(manifest_path, manifest)
            errors = validate(bundle)
            self.assertTrue(any("binary path must be" in error for error in errors), errors)
            self.assertTrue(any("workstation-absolute path leaked" in error for error in errors), errors)

    def test_validator_rejects_workstation_path_in_raw_log(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            (bundle / "logs" / "run.stderr.txt").write_text(
                "failed under /mnt/c/Users/example/private/worktree\n", encoding="utf-8"
            )
            errors = validate(bundle)
            self.assertTrue(any("workstation-absolute path leaked" in error for error in errors), errors)

    def test_validator_rejects_host_identifier_in_raw_log(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            (bundle / "logs" / "run.stderr.txt").write_text(
                "ran on WIN-C2ANEVBHN6Q\n", encoding="utf-8"
            )
            errors = validate(bundle)
            self.assertTrue(any("workstation-absolute path leaked" in error for error in errors), errors)

    def test_validator_rejects_driver_commit_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="b" * 40, driver_profile="development-normal"
            )
            errors = validate(bundle)
            self.assertTrue(any("driver commit does not match" in error for error in errors), errors)

    def test_validator_rejects_driver_profile_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-debug"
            )
            errors = validate(bundle)
            self.assertTrue(any("driver profile does not match" in error for error in errors), errors)

    def test_validator_rejects_driver_compiler_contract_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary),
                driver_commit="a" * 40,
                driver_profile="development-normal",
                driver_compiler_flags="-O0",
            )
            errors = validate(bundle)
            self.assertTrue(
                any("driver compiler_flags does not match" in error for error in errors), errors
            )

    def test_validator_rejects_distribution_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary),
                driver_commit="a" * 40,
                driver_profile="development-normal",
                record_distribution="clustered",
            )
            errors = validate(bundle)
            self.assertTrue(any("input distribution does not match" in error for error in errors), errors)

    def test_validator_rejects_exact_input_distribution_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary),
                driver_commit="a" * 40,
                driver_profile="development-normal",
                artifact_distribution="clustered",
            )
            errors = validate(bundle)
            self.assertTrue(
                any("exact-input distribution does not match" in error for error in errors), errors
            )

    def test_validator_recomputes_summary_json_from_raw_records(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            summary_path = bundle / "summary.json"
            summary = json.loads(summary_path.read_text(encoding="utf-8"))
            summary["cases"][0]["statistics"]["compute_total_seconds"]["median"] = 99.0
            write_json(summary_path, summary)
            errors = validate(bundle)
            self.assertTrue(
                any("summary.json does not match statistics recomputed" in error for error in errors),
                errors,
            )

    def test_validator_recomputes_summary_csv_from_raw_records(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            summary_csv = bundle / "summary.csv"
            summary_csv.write_text(
                summary_csv.read_text(encoding="utf-8").replace("0.03", "99.0"),
                encoding="utf-8",
            )
            errors = validate(bundle)
            self.assertTrue(
                any("summary.csv does not match statistics recomputed" in error for error in errors),
                errors,
            )

    def test_validator_recomputes_distribution_summary_from_raw_records(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            bundle = make_bundle(
                Path(temporary), driver_commit="a" * 40, driver_profile="development-normal"
            )
            distribution_csv = bundle / "distribution-summary.csv"
            distribution_csv.write_text(
                distribution_csv.read_text(encoding="utf-8").replace("0.03", "99.0"),
                encoding="utf-8",
            )
            errors = validate(bundle)
            self.assertTrue(
                any(
                    "distribution summary does not match statistics recomputed" in error
                    for error in errors
                ),
                errors,
            )

    def test_runner_preflight_rejects_compiler_contract_mismatches(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_path = Path(temporary)
            fake_binary = temporary_path / "fake-driver"
            commit = subprocess.check_output(
                ["git", "rev-parse", "HEAD"], cwd=REPO, text=True
            ).strip()
            dirty = bool(subprocess.check_output(
                ["git", "status", "--porcelain=v1", "--untracked-files=all"],
                cwd=REPO,
                text=True,
            ).strip())
            compiler_version = subprocess.check_output(
                ["g++", "--version"], text=True
            ).splitlines()[0].strip()
            expected_build = {
                "commit": commit,
                "dirty": dirty,
                "profile": "development-normal",
                "compiler_command": "g++",
                "compiler_version": compiler_version,
                "compiler_flags": "-O2",
            }
            mismatches = {
                "compiler_command": "not-g++",
                "compiler_version": "not-the-compiler-version",
                "compiler_flags": "-O0",
            }
            for field, bad_value in mismatches.items():
                with self.subTest(field=field):
                    build = {**expected_build, field: bad_value}
                    payload = json.dumps({"status": "ok", "build": build}, sort_keys=True)
                    fake_binary.write_text(
                        "#!/usr/bin/env python3\n"
                        f"print({payload!r})\n",
                        encoding="utf-8",
                    )
                    os.chmod(fake_binary, 0o755)
                    run_id = f"compiler-mismatch-{field}"
                    completed = subprocess.run(
                        [
                            sys.executable,
                            str(BENCHMARK_DIR / "run_benchmarks.py"),
                            "--profile=smoke",
                            f"--binary={fake_binary}",
                            "--compiler=g++",
                            "--compiler-flags=-O2",
                            f"--output-root={temporary_path / 'runs'}",
                            f"--run-id={run_id}",
                            "--allow-dirty",
                        ],
                        cwd=REPO,
                        text=True,
                        capture_output=True,
                    )
                    self.assertEqual(completed.returncode, 2, completed.stdout + completed.stderr)
                    self.assertIn(field, completed.stderr)
                    self.assertFalse((temporary_path / "runs" / run_id).exists())

    def test_runner_preflight_rejects_stale_binary(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_path = Path(temporary)
            fake_binary = temporary_path / "fake-driver"
            dirty = bool(subprocess.check_output(
                ["git", "status", "--porcelain=v1", "--untracked-files=all"],
                cwd=REPO,
                text=True,
            ).strip())
            fake_binary.write_text(
                "#!/usr/bin/env python3\n"
                "import json\n"
                f"print(json.dumps({{'status': 'ok', 'build': {{'commit': {'0' * 40!r}, "
                f"'dirty': {dirty!r}, 'profile': 'development-normal'}}}}))\n",
                encoding="utf-8",
            )
            os.chmod(fake_binary, 0o755)
            completed = subprocess.run(
                [
                    sys.executable,
                    str(BENCHMARK_DIR / "run_benchmarks.py"),
                    "--profile=smoke",
                    f"--binary={fake_binary}",
                    "--compiler=g++",
                    "--compiler-flags=-O2",
                    f"--output-root={temporary_path / 'runs'}",
                    "--run-id=must-not-exist",
                    "--allow-dirty",
                ],
                cwd=REPO,
                text=True,
                capture_output=True,
            )
            self.assertEqual(completed.returncode, 2, completed.stdout + completed.stderr)
            self.assertIn("stale or incoherent", completed.stderr)
            self.assertFalse((temporary_path / "runs" / "must-not-exist").exists())


if __name__ == "__main__":
    unittest.main()
