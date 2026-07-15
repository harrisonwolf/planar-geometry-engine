#!/usr/bin/env python3
"""Render a dependency-free, deterministic SVG from a benchmark summary."""

from __future__ import annotations

import argparse
import html
import json
from pathlib import Path


COLORS = {
    "triangulation": "#2563eb",
    "voronoi": "#14b8a6",
    "validation": "#f59e0b",
}


def render(summary_path: Path, output_path: Path) -> None:
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    cases = summary["cases"]
    width, height = 960, 560
    left, right, top, bottom = 92, 36, 76, 96
    plot_width = width - left - right
    plot_height = height - top - bottom

    totals = [case["statistics"]["compute_total_seconds"]["median"] or 0.0 for case in cases]
    maximum = max(totals, default=1.0) or 1.0
    slot = plot_width / max(1, len(cases))
    bar_width = min(116.0, slot * 0.58)

    lines = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="960" height="560" viewBox="0 0 960 560">',
        '  <rect width="960" height="560" fill="#f8fafc"/>',
        f'  <text x="{left}" y="36" font-family="system-ui,sans-serif" font-size="22" font-weight="700" fill="#0f172a">Planar benchmark phase medians</text>',
        f'  <text x="{left}" y="58" font-family="system-ui,sans-serif" font-size="12" fill="#475569">bundle {html.escape(summary["run_id"])} · successful repetitions only</text>',
        f'  <line x1="{left}" y1="{top + plot_height}" x2="{left + plot_width}" y2="{top + plot_height}" stroke="#64748b"/>',
        f'  <line x1="{left}" y1="{top}" x2="{left}" y2="{top + plot_height}" stroke="#64748b"/>',
    ]

    for tick in range(6):
        value = maximum * tick / 5
        y = top + plot_height - plot_height * tick / 5
        lines.append(f'  <line x1="{left}" y1="{y:.2f}" x2="{left + plot_width}" y2="{y:.2f}" stroke="#e2e8f0"/>')
        lines.append(f'  <text x="{left - 10}" y="{y + 4:.2f}" text-anchor="end" font-family="ui-monospace,monospace" font-size="11" fill="#475569">{value:.4f}s</text>')

    for index, case in enumerate(cases):
        center = left + slot * (index + 0.5)
        x = center - bar_width / 2
        y_cursor = top + plot_height
        for phase in ("triangulation", "voronoi", "validation"):
            median = case["statistics"][f"{phase}_seconds"]["median"] or 0.0
            segment_height = plot_height * median / maximum
            y_cursor -= segment_height
            lines.append(
                f'  <rect x="{x:.2f}" y="{y_cursor:.2f}" width="{bar_width:.2f}" height="{segment_height:.2f}" fill="{COLORS[phase]}"><title>{phase}: {median:.9f}s</title></rect>'
            )
        total = case["statistics"]["compute_total_seconds"]["median"]
        total_label = "n/a" if total is None else f"{total:.6f}s"
        lines.append(f'  <text x="{center:.2f}" y="{max(top + 12, y_cursor - 8):.2f}" text-anchor="middle" font-family="ui-monospace,monospace" font-size="11" fill="#0f172a">{total_label}</text>')
        lines.append(f'  <text x="{center:.2f}" y="{top + plot_height + 23}" text-anchor="middle" font-family="system-ui,sans-serif" font-size="12" fill="#0f172a">{html.escape(case["case_id"])}</text>')
        lines.append(f'  <text x="{center:.2f}" y="{top + plot_height + 40}" text-anchor="middle" font-family="system-ui,sans-serif" font-size="10" fill="#64748b">{case["successful_repetitions"]}/{case["planned_repetitions"]} ok</text>')

    legend_x = left
    for phase in ("triangulation", "voronoi", "validation"):
        lines.append(f'  <rect x="{legend_x}" y="525" width="12" height="12" rx="2" fill="{COLORS[phase]}"/>')
        lines.append(f'  <text x="{legend_x + 18}" y="535" font-family="system-ui,sans-serif" font-size="12" fill="#334155">{phase}</text>')
        legend_x += 150

    lines.append('</svg>')
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("summary", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(args.summary, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
