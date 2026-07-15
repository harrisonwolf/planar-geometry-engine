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
        require(isinstance(record.get("command"), list), f"run {line_number}: command is not an array", errors)
        require(isinstance(record.get("repetition"), int), f"run {line_number}: repetition is not an integer", errors)
        measurements = record.get("measurements", {})
        for field in NUMERIC_OR_NULL:
            value = measurements.get(field)
            require(value is None or (isinstance(value, (int, float)) and math.isfinite(value)),
                    f"run {line_number}: {field} must be finite numeric/null", errors)
        for relative in record.get("raw_artifacts", {}).values():
            require((bundle / relative).is_file(), f"run {line_number}: missing {relative}", errors)

    require(len(records) == manifest.get("run_count"), "manifest run_count mismatch", errors)
    with (bundle / "summary.csv").open(newline="", encoding="utf-8") as handle:
        require(len(list(csv.DictReader(handle))) > 0, "summary.csv has no cases", errors)

    expected: dict[str, str] = {}
    for line in (bundle / "checksums.sha256").read_text(encoding="utf-8").splitlines():
        checksum, relative = line.split("  ", 1)
        expected[relative] = checksum
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
