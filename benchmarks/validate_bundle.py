#!/usr/bin/env python3
"""Validate a schema-v1 Planar benchmark bundle and its checksums."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import re
from pathlib import Path


STATUSES = {"ok", "timeout", "oom", "error", "skipped", "validation_failed"}
NUMERIC_OR_NULL = {
    "external_wall_seconds", "user_seconds", "system_seconds", "max_rss_kb",
    "input_generation_seconds", "triangulation_seconds", "voronoi_seconds",
    "validation_seconds", "compute_total_seconds",
}
WORKSTATION_PATH_PATTERN = re.compile(
    r"(?:/home/[^/\s\"']+|/Users/[^/\s\"']+|/mnt/[A-Za-z]/Users/[^/\s\"']+|[A-Za-z]:[\\\\/]Users[\\\\/][^\\\\/\s\"']+|\bWIN-[A-Z0-9]{6,}\b)",
    re.IGNORECASE,
)


def is_absolute_path_text(value: object) -> bool:
    return isinstance(value, str) and (
        value.startswith("/") or re.match(r"^[A-Za-z]:[\\\\/]", value) is not None
    )


def is_safe_relative_path(value: object) -> bool:
    if not isinstance(value, str) or not value or is_absolute_path_text(value):
        return False
    return ".." not in Path(value).parts


def workstation_path_leaks(bundle: Path) -> list[str]:
    leaks: list[str] = []
    for path in sorted(item for item in bundle.rglob("*") if item.is_file()):
        try:
            content = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        for line_number, line in enumerate(content.splitlines(), 1):
            if WORKSTATION_PATH_PATTERN.search(line):
                leaks.append(
                    f"workstation-absolute path leaked in {path.relative_to(bundle)}:{line_number}"
                )
    return leaks


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
    errors.extend(workstation_path_leaks(bundle))

    manifest = json.loads((bundle / "manifest.json").read_text(encoding="utf-8"))
    require(manifest.get("schema_version") == 1, "manifest schema_version must be 1", errors)
    require(manifest.get("bundle_kind") == "planar_benchmark_run_bundle", "unexpected bundle kind", errors)
    repository = manifest.get("repository", {})
    build_contract = manifest.get("build_contract", {})
    contract_version = build_contract.get("contract_version", 1)
    binary = manifest.get("binary", {})
    binary_path = binary.get("path")
    profile_definition = manifest.get("profile_definition", {})
    profile_cases = profile_definition.get("cases", [])
    case_definitions = {
        case.get("id"): case
        for case in profile_cases
        if isinstance(case, dict) and isinstance(case.get("id"), str)
    }
    require(
        len(case_definitions) == len(profile_cases),
        "manifest profile cases must have unique string ids",
        errors,
    )
    require(isinstance(repository.get("commit"), str) and len(repository.get("commit", "")) == 40,
            "manifest repository commit must be a full SHA", errors)
    require(isinstance(repository.get("dirty"), bool), "manifest repository dirty must be boolean", errors)
    require(bool(build_contract.get("compiler")), "manifest compiler is missing", errors)
    require(bool(build_contract.get("compiler_flags")), "manifest compiler flags are missing", errors)
    require(contract_version in (1, 2), "unsupported build contract version", errors)
    if contract_version == 2:
        require(
            build_contract.get("compiler") == build_contract.get("compiler_command"),
            "manifest compiler aliases disagree",
            errors,
        )
        for field in ("compiler_command", "compiler_version", "compiler_flags"):
            require(
                bool(build_contract.get(field)),
                f"manifest build contract is missing {field}",
                errors,
            )
    require(bool(build_contract.get("expected_driver_profile")), "expected driver profile is missing", errors)
    require(build_contract.get("provenance_preflight_passed") is True,
            "runner provenance preflight did not pass", errors)
    require(isinstance(binary.get("sha256"), str) and len(binary.get("sha256", "")) == 64,
            "manifest binary SHA-256 is missing", errors)
    require(is_safe_relative_path(binary_path),
            "manifest binary path must be a repository-relative logical path", errors)
    require(manifest.get("working_directory") == "$REPO_ROOT",
            "manifest working_directory must use $REPO_ROOT", errors)
    runner_command = manifest.get("runner_command")
    require(
        isinstance(runner_command, list)
        and len(runner_command) >= 2
        and runner_command[:2] == ["python3", "benchmarks/run_benchmarks.py"]
        and not any(is_absolute_path_text(part) for part in runner_command),
        "manifest runner_command must use publication-safe logical paths",
        errors,
    )
    environment = json.loads((bundle / "environment.json").read_text(encoding="utf-8"))
    require(environment.get("repository_path") == "$REPO_ROOT",
            "environment repository_path must use $REPO_ROOT", errors)
    require("PATH" not in environment.get("environment_variables", {}),
            "environment must not record PATH", errors)
    require("uname" not in environment.get("platform", {}),
            "environment must not record hostname-bearing uname output", errors)

    records = []
    for line_number, line in enumerate((bundle / "runs.jsonl").read_text(encoding="utf-8").splitlines(), 1):
        try:
            record = json.loads(line)
        except json.JSONDecodeError as exc:
            errors.append(f"runs.jsonl:{line_number}: {exc}")
            continue
        records.append(record)
        case_id = record.get("case_id")
        case_definition = case_definitions.get(case_id)
        if profile_cases:
            require(
                case_definition is not None,
                f"run {line_number}: case_id is not present in manifest profile",
                errors,
            )
        run_input = record.get("input", {})
        if case_definition is not None:
            expected_identity = {
                "seed": case_definition.get("seed"),
                "requested_count": case_definition.get("count"),
                "coord_max": case_definition.get("coord_max"),
            }
            if "distribution" in case_definition:
                expected_identity["distribution"] = case_definition["distribution"]
            for field, expected_value in expected_identity.items():
                require(
                    run_input.get(field) == expected_value,
                    f"run {line_number}: input {field} does not match manifest profile",
                    errors,
                )
            artifact_relative = run_input.get("points_artifact")
            if artifact_relative:
                artifact_path = bundle / artifact_relative
                require(
                    artifact_path.is_file(),
                    f"run {line_number}: missing exact-input artifact {artifact_relative}",
                    errors,
                )
                if artifact_path.is_file():
                    artifact_input = json.loads(artifact_path.read_text(encoding="utf-8"))
                    require(
                        sha256(artifact_path) == run_input.get("points_sha256"),
                        f"run {line_number}: exact-input SHA-256 does not match run record",
                        errors,
                    )
                    expected_artifact_identity = {
                        "seed": case_definition.get("seed"),
                        "coord_max": case_definition.get("coord_max"),
                    }
                    if "distribution" in case_definition:
                        expected_artifact_identity["distribution"] = case_definition["distribution"]
                    for field, expected_value in expected_artifact_identity.items():
                        require(
                            artifact_input.get(field) == expected_value,
                            f"run {line_number}: exact-input {field} does not match manifest profile",
                            errors,
                        )
                    points = artifact_input.get("points")
                    valid_points = (
                        isinstance(points, list)
                        and all(
                            isinstance(point, list)
                            and len(point) == 2
                            and all(isinstance(coordinate, int) for coordinate in point)
                            for point in points
                        )
                    )
                    require(valid_points, f"run {line_number}: exact points are malformed", errors)
                    if valid_points:
                        require(
                            len(points) == case_definition.get("count"),
                            f"run {line_number}: exact point count does not match manifest profile",
                            errors,
                        )
                        require(
                            len({tuple(point) for point in points}) == len(points),
                            f"run {line_number}: exact points are not unique",
                            errors,
                        )
                        bound = case_definition.get("coord_max")
                        require(
                            isinstance(bound, int)
                            and all(
                                -bound <= coordinate <= bound
                                for point in points
                                for coordinate in point
                            ),
                            f"run {line_number}: exact point exceeds manifest coordinate bound",
                            errors,
                        )
        require(record.get("schema_version") == 1, f"run {line_number}: bad schema", errors)
        require(record.get("status") in STATUSES, f"run {line_number}: bad status", errors)
        command = record.get("command")
        require(isinstance(command, list), f"run {line_number}: command is not an array", errors)
        if isinstance(command, list) and command:
            require(command[0] == binary.get("path"),
                    f"run {line_number}: command binary does not match manifest", errors)
            require(not any(is_absolute_path_text(part) for part in command),
                    f"run {line_number}: command contains an absolute path", errors)
        driver_build = record.get("driver_build", {})
        require(driver_build.get("commit") == repository.get("commit"),
                f"run {line_number}: driver commit does not match manifest repository commit", errors)
        require(driver_build.get("dirty") == repository.get("dirty"),
                f"run {line_number}: driver dirty state does not match manifest", errors)
        require(driver_build.get("profile") == build_contract.get("expected_driver_profile"),
                f"run {line_number}: driver profile does not match build contract", errors)
        if contract_version == 2:
            for field in ("compiler_command", "compiler_version", "compiler_flags"):
                require(
                    driver_build.get(field) == build_contract.get(field),
                    f"run {line_number}: driver {field} does not match build contract",
                    errors,
                )
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
            require(is_safe_relative_path(relative),
                    f"run {line_number}: unsafe raw artifact path {relative}", errors)
            if is_safe_relative_path(relative):
                require((bundle / relative).is_file(), f"run {line_number}: missing {relative}", errors)

    require(len(records) == manifest.get("run_count"), "manifest run_count mismatch", errors)
    preflight_build = build_contract.get("preflight_driver_build", {})
    require(preflight_build.get("commit") == repository.get("commit"),
            "preflight driver commit does not match manifest repository", errors)
    require(preflight_build.get("dirty") == repository.get("dirty"),
            "preflight driver dirty state does not match manifest", errors)
    require(preflight_build.get("profile") == build_contract.get("expected_driver_profile"),
            "preflight driver profile does not match build contract", errors)
    if contract_version == 2:
        for field in ("compiler_command", "compiler_version", "compiler_flags"):
            require(
                preflight_build.get(field) == build_contract.get(field),
                f"preflight driver {field} does not match build contract",
                errors,
            )
    with (bundle / "summary.csv").open(newline="", encoding="utf-8") as handle:
        require(len(list(csv.DictReader(handle))) > 0, "summary.csv has no cases", errors)

    expected: dict[str, str] = {}
    for artifact_key in ("distribution_summary_csv", "distribution_chart"):
        declared_artifact = manifest.get("artifacts", {}).get(artifact_key)
        if declared_artifact:
            require(
                (bundle / declared_artifact).is_file(),
                f"missing declared {artifact_key}: {declared_artifact}",
                errors,
            )
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
