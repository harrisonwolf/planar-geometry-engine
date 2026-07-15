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
    "compute_total": "#0f172a",
}


def render(summary_path: Path, output_path: Path) -> None:
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    cases = summary["cases"]
    width, height = 960, 560
    left, right, top, bottom = 92, 36, 94, 96
    plot_width = width - left - right
    plot_height = height - top - bottom

    totals = [case["statistics"]["compute_total_seconds"]["median"] or 0.0 for case in cases]
    maximum = max(totals, default=1.0) or 1.0
    slot = plot_width / max(1, len(cases))
    group_width = min(150.0, slot * 0.76)
    metrics = ("triangulation", "voronoi", "validation", "compute_total")
    bar_gap = 3.0
    bar_width = (group_width - bar_gap * (len(metrics) - 1)) / len(metrics)

    lines = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="960" height="560" viewBox="0 0 960 560">',
        '  <rect width="960" height="560" fill="#f8fafc"/>',
        f'  <text x="{left}" y="36" font-family="system-ui,sans-serif" font-size="22" font-weight="700" fill="#0f172a">Planar phase and total medians</text>',
        f'  <text x="{left}" y="58" font-family="system-ui,sans-serif" font-size="12" fill="#475569">bundle {html.escape(summary["run_id"])} · successful repetitions only</text>',
        f'  <text x="{left}" y="76" font-family="system-ui,sans-serif" font-size="10" fill="#64748b">Each bar is an independent median; phase medians are not stacked because medians are not additive.</text>',
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
        group_x = center - group_width / 2
        total_center = center
        total_y = top + plot_height
        for metric_index, metric in enumerate(metrics):
            median = case["statistics"][f"{metric}_seconds"]["median"] or 0.0
            bar_height = plot_height * median / maximum
            x = group_x + metric_index * (bar_width + bar_gap)
            y = top + plot_height - bar_height
            label = "total" if metric == "compute_total" else metric
            lines.append(
                f'  <rect x="{x:.2f}" y="{y:.2f}" width="{bar_width:.2f}" height="{bar_height:.2f}" fill="{COLORS[metric]}"><title>{label} median: {median:.9f}s</title></rect>'
            )
            if metric == "compute_total":
                total_center = x + bar_width / 2
                total_y = y
        total = case["statistics"]["compute_total_seconds"]["median"]
        total_label = "n/a" if total is None else f"{total:.6f}s"
        lines.append(f'  <text x="{total_center:.2f}" y="{max(top + 12, total_y - 8):.2f}" text-anchor="middle" font-family="ui-monospace,monospace" font-size="11" fill="#0f172a">total {total_label}</text>')
        lines.append(f'  <text x="{center:.2f}" y="{top + plot_height + 23}" text-anchor="middle" font-family="system-ui,sans-serif" font-size="12" fill="#0f172a">{html.escape(case["case_id"])}</text>')
        lines.append(f'  <text x="{center:.2f}" y="{top + plot_height + 40}" text-anchor="middle" font-family="system-ui,sans-serif" font-size="10" fill="#64748b">{case["successful_repetitions"]}/{case["planned_repetitions"]} ok</text>')

    legend_x = left
    for metric in metrics:
        label = "compute total" if metric == "compute_total" else metric
        lines.append(f'  <rect x="{legend_x}" y="525" width="12" height="12" rx="2" fill="{COLORS[metric]}"/>')
        lines.append(f'  <text x="{legend_x + 18}" y="535" font-family="system-ui,sans-serif" font-size="12" fill="#334155">{label}</text>')
        legend_x += 150

    lines.append('</svg>')
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def render_distributions(summary_path: Path, output_path: Path) -> None:
    """Render across-seed distribution medians and explicit uncertainty."""
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    aggregates = summary.get("distribution_aggregates", [])
    width, height = 960, 560
    left, right, top, bottom = 92, 36, 96, 120
    plot_width = width - left - right
    plot_height = height - top - bottom

    if not aggregates:
        lines = [
            '<svg xmlns="http://www.w3.org/2000/svg" width="960" height="560" viewBox="0 0 960 560">',
            '  <rect width="960" height="560" fill="#f8fafc"/>',
            f'  <text x="92" y="42" font-family="system-ui,sans-serif" font-size="22" font-weight="700" fill="#0f172a">Planar distribution comparison</text>',
            f'  <text x="92" y="72" font-family="system-ui,sans-serif" font-size="13" fill="#475569">bundle {html.escape(summary["run_id"])}</text>',
            '  <text x="92" y="150" font-family="system-ui,sans-serif" font-size="16" fill="#475569">This profile does not define across-seed distribution aggregates.</text>',
            '</svg>',
        ]
        output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
        return

    statistics = [
        aggregate["compute_total_seconds_across_seed_case_medians"]
        for aggregate in aggregates
    ]
    maximum = max(
        (statistic["maximum"] or statistic["median"] or 0.0)
        for statistic in statistics
    ) or 1.0
    maximum *= 1.12
    slot = plot_width / len(aggregates)
    site_counts = sorted({
        int(case["input"]["count"])
        for case in summary["cases"]
    })
    site_label = (
        f'n={site_counts[0]:,} sites'
        if len(site_counts) == 1
        else "mixed site counts"
    )
    seed_counts = sorted({int(item["distinct_seed_count"]) for item in aggregates})
    repetitions = sorted({
        int(item["planned_run_count"]) // max(1, int(item["case_count"]))
        for item in aggregates
    })
    seed_label = (
        f"{seed_counts[0]} seeds/distribution"
        if len(seed_counts) == 1
        else "mixed seed counts"
    )
    repetition_label = (
        f"{repetitions[0]} repetitions/input"
        if len(repetitions) == 1
        else "mixed repetitions"
    )

    def y_position(value: float) -> float:
        return top + plot_height - plot_height * value / maximum

    lines = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="960" height="560" viewBox="0 0 960 560">',
        '  <rect width="960" height="560" fill="#f8fafc"/>',
        f'  <text x="{left}" y="36" font-family="system-ui,sans-serif" font-size="22" font-weight="700" fill="#0f172a">Planar compute time by input distribution</text>',
        f'  <text x="{left}" y="60" font-family="system-ui,sans-serif" font-size="12" fill="#475569">bundle {html.escape(summary["run_id"])} · {site_label} · {seed_label} · {repetition_label}</text>',
        f'  <text x="{left}" y="78" font-family="system-ui,sans-serif" font-size="11" fill="#64748b">Median dot and MAD box; dark whisker is p10–p90 and pale whisker is min–max across seed-level case medians.</text>',
        f'  <line x1="{left}" y1="{top + plot_height}" x2="{left + plot_width}" y2="{top + plot_height}" stroke="#64748b"/>',
        f'  <line x1="{left}" y1="{top}" x2="{left}" y2="{top + plot_height}" stroke="#64748b"/>',
    ]
    for tick in range(6):
        value = maximum * tick / 5
        y = y_position(value)
        lines.append(f'  <line x1="{left}" y1="{y:.2f}" x2="{left + plot_width}" y2="{y:.2f}" stroke="#e2e8f0"/>')
        lines.append(f'  <text x="{left - 10}" y="{y + 4:.2f}" text-anchor="end" font-family="ui-monospace,monospace" font-size="11" fill="#475569">{value:.4f}s</text>')

    for index, (aggregate, statistic) in enumerate(zip(aggregates, statistics)):
        center = left + slot * (index + 0.5)
        median = float(statistic["median"] or 0.0)
        mad = float(statistic["mad"] or 0.0)
        minimum = float(statistic["minimum"] or median)
        maximum_value = float(statistic["maximum"] or median)
        p10 = float(statistic["p10"] or median)
        p90 = float(statistic["p90"] or median)
        lines.append(f'  <line x1="{center:.2f}" y1="{y_position(maximum_value):.2f}" x2="{center:.2f}" y2="{y_position(minimum):.2f}" stroke="#94a3b8" stroke-width="3"/>')
        lines.append(f'  <line x1="{center:.2f}" y1="{y_position(p90):.2f}" x2="{center:.2f}" y2="{y_position(p10):.2f}" stroke="#2563eb" stroke-width="7"/>')
        band_top = y_position(median + mad)
        band_bottom = y_position(max(0.0, median - mad))
        lines.append(f'  <rect x="{center - 22:.2f}" y="{band_top:.2f}" width="44" height="{max(2.0, band_bottom - band_top):.2f}" rx="4" fill="#bfdbfe" stroke="#2563eb"/>')
        lines.append(f'  <circle cx="{center:.2f}" cy="{y_position(median):.2f}" r="6" fill="#0f172a"/>')
        lines.append(f'  <text x="{center:.2f}" y="{max(top + 12, y_position(maximum_value) - 10):.2f}" text-anchor="middle" font-family="ui-monospace,monospace" font-size="11" fill="#0f172a">{median:.6f}s</text>')
        lines.append(f'  <text x="{center:.2f}" y="{top + plot_height + 25}" text-anchor="middle" font-family="system-ui,sans-serif" font-size="13" font-weight="600" fill="#0f172a">{html.escape(aggregate["distribution"])}</text>')
        lines.append(f'  <text x="{center:.2f}" y="{top + plot_height + 44}" text-anchor="middle" font-family="system-ui,sans-serif" font-size="10" fill="#64748b">{aggregate["successful_run_count"]}/{aggregate["planned_run_count"]} runs ok</text>')

    lines.append(f'  <text x="{left}" y="540" font-family="system-ui,sans-serif" font-size="11" fill="#64748b">Uncertainty sample: one median per deterministic seed input (not pooled timing repetitions).</text>')
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
