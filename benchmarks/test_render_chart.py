#!/usr/bin/env python3
"""Regression tests for statistically honest benchmark charts."""

from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from render_chart import render


class RenderChartTests(unittest.TestCase):
    def test_phase_medians_are_grouped_with_an_independent_total(self) -> None:
        summary = {
            "run_id": "non-additive-median-fixture",
            "cases": [{
                "case_id": "case-a",
                "successful_repetitions": 4,
                "planned_repetitions": 4,
                "statistics": {
                    "triangulation_seconds": {"median": 0.01},
                    "voronoi_seconds": {"median": 0.02},
                    "validation_seconds": {"median": 0.03},
                    # Deliberately not the sum of the independent phase medians.
                    "compute_total_seconds": {"median": 0.09},
                },
            }],
        }
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            summary_path = root / "summary.json"
            output_path = root / "chart.svg"
            summary_path.write_text(json.dumps(summary), encoding="utf-8")

            render(summary_path, output_path)

            chart = output_path.read_text(encoding="utf-8")
            self.assertIn("phase medians are not stacked because medians are not additive", chart)
            self.assertIn("<title>triangulation median: 0.010000000s</title>", chart)
            self.assertIn("<title>voronoi median: 0.020000000s</title>", chart)
            self.assertIn("<title>validation median: 0.030000000s</title>", chart)
            self.assertIn("<title>total median: 0.090000000s</title>", chart)
            self.assertIn(">total 0.090000s</text>", chart)


if __name__ == "__main__":
    unittest.main()
