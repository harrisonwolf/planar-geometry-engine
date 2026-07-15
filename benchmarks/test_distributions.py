#!/usr/bin/env python3
"""Contract tests for deterministic benchmark input distributions."""

from __future__ import annotations

import json
import subprocess
import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parent.parent
DRIVER = REPO / "build" / "development" / "normal" / "benchmark_driver"
DISTRIBUTIONS = (
    "uniform",
    "clustered",
    "jittered-grid",
    "near-collinear",
)
GENERATOR_NAMES = {
    "uniform": "splitmix64_uniform_unique_integer_v1",
    "clustered": "splitmix64_four_cluster_integer_v1",
    "jittered-grid": "splitmix64_jittered_grid_integer_v1",
    "near-collinear": "splitmix64_near_collinear_integer_v1",
}


def run_driver(distribution: str) -> dict:
    completed = subprocess.run(
        [
            str(DRIVER),
            "--count=128",
            "--seed=20260715",
            "--coord-max=1000000",
            f"--distribution={distribution}",
            "--emit-points",
        ],
        cwd=REPO,
        text=True,
        capture_output=True,
        timeout=30,
    )
    if completed.returncode != 0:
        raise AssertionError(
            f"{distribution} driver failed ({completed.returncode}): {completed.stderr}"
        )
    return json.loads(completed.stdout)


def exact_input_bytes(payload: dict) -> bytes:
    return json.dumps(
        payload["input"], sort_keys=True, separators=(",", ":"), ensure_ascii=True
    ).encode("ascii")


class DistributionGeneratorTests(unittest.TestCase):
    def test_all_distributions_are_deterministic_unique_and_bounded(self) -> None:
        for distribution in DISTRIBUTIONS:
            with self.subTest(distribution=distribution):
                first = run_driver(distribution)
                second = run_driver(distribution)
                self.assertEqual(exact_input_bytes(first), exact_input_bytes(second))

                driver_input = first["input"]
                points = [tuple(point) for point in driver_input["points"]]
                self.assertEqual(driver_input["distribution"], distribution)
                self.assertEqual(driver_input["generator"], GENERATOR_NAMES[distribution])
                self.assertEqual(driver_input["requested_count"], 128)
                self.assertEqual(len(points), 128)
                self.assertEqual(len(set(points)), 128)
                self.assertTrue(all(
                    -1000000 <= coordinate <= 1000000
                    for point in points
                    for coordinate in point
                ))
                self.assertEqual(first["topology"]["sites"], 128)
                self.assertTrue(first["validation"]["all_passed"])

    def test_near_collinear_family_is_not_exactly_collinear(self) -> None:
        points = run_driver("near-collinear")["input"]["points"]
        first = points[0]
        last = points[-1]
        cross_products = [
            (last[0] - first[0]) * (point[1] - first[1])
            - (last[1] - first[1]) * (point[0] - first[0])
            for point in points[1:-1]
        ]
        self.assertTrue(any(value != 0 for value in cross_products))


if __name__ == "__main__":
    unittest.main()
