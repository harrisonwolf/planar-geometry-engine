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

from validate_bundle import validate  # noqa: E402


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def make_bundle(root: Path, *, driver_commit: str, driver_profile: str) -> Path:
    bundle = root / "bundle"
    (bundle / "logs").mkdir(parents=True)
    (bundle / "charts").mkdir()
    commit = "a" * 40
    binary_path = "/tmp/planar-benchmark-driver"
    manifest = {
        "schema_version": 1,
        "bundle_kind": "planar_benchmark_run_bundle",
        "repository": {"commit": commit, "dirty": False},
        "binary": {"path": binary_path, "sha256": "c" * 64},
        "build_contract": {
            "compiler": "g++",
            "compiler_flags": "-Wall -Wextra -std=c++17 -O2",
            "expected_driver_profile": "development-normal",
            "preflight_driver_build": {
                "commit": commit,
                "dirty": False,
                "profile": "development-normal",
            },
            "provenance_preflight_passed": True,
        },
        "run_count": 1,
    }
    record = {
        "schema_version": 1,
        "run_id": "test",
        "case_id": "n10",
        "repetition": 1,
        "status": "ok",
        "command": [binary_path, "--count=10"],
        "timeout_seconds": 10.0,
        "exit_code": 0,
        "driver_build": {
            "commit": driver_commit,
            "dirty": False,
            "profile": driver_profile,
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
        "input": {},
        "output": {},
        "validation": {},
        "raw_artifacts": {
            "stdout": "logs/run.stdout.json",
            "stderr": "logs/run.stderr.txt",
            "resource_time": "logs/run.time.txt",
        },
    }
    write_json(bundle / "manifest.json", manifest)
    write_json(bundle / "environment.json", {})
    (bundle / "runs.jsonl").write_text(json.dumps(record, sort_keys=True) + "\n", encoding="utf-8")
    write_json(bundle / "summary.json", {})
    (bundle / "summary.csv").write_text("case_id,ok\nn10,1\n", encoding="utf-8")
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
