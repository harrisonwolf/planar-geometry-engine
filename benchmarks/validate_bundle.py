#!/usr/bin/env python3
"""Validate a schema-v1 Planar benchmark bundle and its checksums."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path


STATUSES = {"ok", "timeout", "oom", "error", "skipped", "validation_failed"}
NUMERIC_OR_NULL = {
    "external_wall_seconds", "user_seconds", "system_seconds", "max_rss_kb",
    "input_generation_seconds", "triangulation_seconds", "voronoi_seconds",
    "validation_seconds", "compute_total_seconds",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def validate(bundle: Path) -> list[str]:
    errors: list[str] = []
    required = [
        "manifest.json", "environment.json", "runs.jsonl", "summary.json",
        "summary.csv", "validation.json", "checksums.sha256", "charts/phase-medians.svg",
    ]
    for relative in required:
        require((bundle / relative).is_file(), f"missing {relative}", errors)
    if errors:
        return errors

    manifest = json.loads((bundle / "manifest.json").read_text(encoding="utf-8"))
    require(manifest.get("schema_version") == 1, "manifest schema_version must be 1", errors)
    require(manifest.get("bundle_kind") == "planar_benchmark_run_bundle", "unexpected bundle kind", errors)
    repository = manifest.get("repository", {})
    build_contract = manifest.get("build_contract", {})
    binary = manifest.get("binary", {})
    require(isinstance(repository.get("commit"), str) and len(repository.get("commit", "")) == 40,
            "manifest repository commit must be a full SHA", errors)
    require(isinstance(repository.get("dirty"), bool), "manifest repository dirty must be boolean", errors)
    require(bool(build_contract.get("compiler")), "manifest compiler is missing", errors)
    require(bool(build_contract.get("compiler_flags")), "manifest compiler flags are missing", errors)
    require(bool(build_contract.get("expected_driver_profile")), "expected driver profile is missing", errors)
    require(build_contract.get("provenance_preflight_passed") is True,
            "runner provenance preflight did not pass", errors)
    require(isinstance(binary.get("sha256"), str) and len(binary.get("sha256", "")) == 64,
            "manifest binary SHA-256 is missing", errors)

    records = []
    for line_number, line in enumerate((bundle / "runs.jsonl").read_text(encoding="utf-8").splitlines(), 1):
        try:
            record = json.loads(line)
        except json.JSONDecodeError as exc:
            errors.append(f"runs.jsonl:{line_number}: {exc}")
            continue
        records.append(record)
        require(record.get("schema_version") == 1, f"run {line_number}: bad schema", errors)
        require(record.get("status") in STATUSES, f"run {line_number}: bad status", errors)
        command = record.get("command")
        require(isinstance(command, list), f"run {line_number}: command is not an array", errors)
        if isinstance(command, list) and command:
            require(command[0] == binary.get("path"),
                    f"run {line_number}: command binary does not match manifest", errors)
        driver_build = record.get("driver_build", {})
        require(driver_build.get("commit") == repository.get("commit"),
                f"run {line_number}: driver commit does not match manifest repository commit", errors)
        require(driver_build.get("dirty") == repository.get("dirty"),
                f"run {line_number}: driver dirty state does not match manifest", errors)
        require(driver_build.get("profile") == build_contract.get("expected_driver_profile"),
                f"run {line_number}: driver profile does not match build contract", errors)
        if repository.get("dirty") is False:
            require(driver_build.get("dirty") is False,
                    f"run {line_number}: clean manifest contains dirty driver", errors)
        require(isinstance(record.get("repetition"), int), f"run {line_number}: repetition is not an integer", errors)
        measurements = record.get("measurements", {})
        for field in NUMERIC_OR_NULL:
            value = measurements.get(field)
            require(value is None or (isinstance(value, (int, float)) and math.isfinite(value)),
                    f"run {line_number}: {field} must be finite numeric/null", errors)
        for relative in record.get("raw_artifacts", {}).values():
            require((bundle / relative).is_file(), f"run {line_number}: missing {relative}", errors)

    require(len(records) == manifest.get("run_count"), "manifest run_count mismatch", errors)
    preflight_build = build_contract.get("preflight_driver_build", {})
    require(preflight_build.get("commit") == repository.get("commit"),
            "preflight driver commit does not match manifest repository", errors)
    require(preflight_build.get("dirty") == repository.get("dirty"),
            "preflight driver dirty state does not match manifest", errors)
    require(preflight_build.get("profile") == build_contract.get("expected_driver_profile"),
            "preflight driver profile does not match build contract", errors)
    with (bundle / "summary.csv").open(newline="", encoding="utf-8") as handle:
        require(len(list(csv.DictReader(handle))) > 0, "summary.csv has no cases", errors)

    expected: dict[str, str] = {}
    for line in (bundle / "checksums.sha256").read_text(encoding="utf-8").splitlines():
        checksum, relative = line.split("  ", 1)
        expected[relative] = checksum
    actual_files = {
        path.relative_to(bundle).as_posix()
        for path in bundle.rglob("*")
        if path.is_file() and path.name != "checksums.sha256"
    }
    require(set(expected) == actual_files, "checksum inventory does not match bundle files", errors)
    for relative, checksum in expected.items():
        path = bundle / relative
        require(path.is_file(), f"checksum entry missing file: {relative}", errors)
        if path.is_file():
            require(sha256(path) == checksum, f"checksum mismatch: {relative}", errors)
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("bundle", type=Path)
    args = parser.parse_args()
    errors = validate(args.bundle)
    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1
    print(f"validated schema-v1 bundle: {args.bundle}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
