# Reproducible Benchmarks

The benchmark surface is separate from the interactive Delaunay driver and the portfolio figure exporter. Its purpose is to make timing claims independently inspectable: the input is fixed, compute phases are explicit, every process has a wall timeout, and every published summary can be traced back to raw output.

## Quick Start

```bash
make benchmark-smoke
```

The command builds the optimized `benchmark_driver`, runs the smoke profile, creates a new immutable directory under `benchmarks/runs/`, validates its schema and checksums, and prints the final bundle path. A generated bundle is never overwritten. `RUN_ID=my-id` may be supplied when a stable identifier is useful; it must still be new.

Validate an existing bundle without modifying it:

```bash
make benchmark-validate BUNDLE=benchmarks/runs/<run-id>
```

Run the benchmark-tooling provenance regressions:

```bash
make benchmark-tools-test
```

## Metric Definition

The dedicated C++ driver uses versioned deterministic SplitMix64 integer-point generators. Distribution, generator version, seed, count, coordinate bound, and the exact point list are captured as input identity. Input creation finishes before the geometry phases begin.

The primary `compute_total_seconds` metric is the sum of steady-clock measurements for:

1. Bowyer-Watson triangulation
2. finite Voronoi construction
3. Delaunay, adjacency, Euler, and dual-count validation

It excludes deterministic input generation, JSON serialization, and process startup. The records also preserve each phase separately. `external_wall_seconds` measures the full child-process invocation, and GNU `time` supplies user CPU time, system CPU time, and peak resident memory. These metrics are not interchangeable, so published numbers must name the field they use.

Every successful repetition must produce the same exact-input SHA-256 and canonical triangulation hash. A mismatch is recorded as a bundle validation failure rather than averaged into the timing result.

## Profiles

Profile definitions are versioned in `benchmarks/profiles.json`.

| Profile | Cases | Repetitions | Per-run timeout | Intended use |
| --- | --- | ---: | ---: | --- |
| `smoke` | 50 and 100 sites | 3 | 15 s | schema, correctness, and tooling checks |
| `standard` | 100, 300, 1,000, 1,500, and 2,500 sites | 10 | 120 s | scaling and routine publication refreshes |
| `headline` | fixed 1,500-site input | 30 | 120 s | a statistically stable portfolio headline |
| research | 4 distributions × 3 seeds at 1,024 sites | 5 per exact input | 120 s | bounded input-shape and seed-sensitivity study |

Run the larger profiles explicitly:

```bash
make benchmark-standard
make benchmark-headline
make benchmark-research
```

Do not treat a profile name as a performance guarantee. Runtime depends on the machine, compiler, power state, and competing work. The current Delaunay validator checks every triangle against every site, so validation has quadratic scaling and is intentionally visible as its own phase.

### Research input families

The research profile holds site count and coordinate bound fixed at 1,024 and ±1,000,000. It creates three deterministic inputs per family (seeds 20260715–20260717) and times each exact input five times, for 60 planned runs. Seeds sample input sensitivity; repetitions sample run-to-run timing noise on the same input. They are not interchangeable.

| Family | Construction | Evidence it contributes | Boundary of the evidence |
| --- | --- | --- | --- |
| uniform | unique integer points across the full square | baseline behavior on globally unstructured inputs and sensitivity to three fixed seeds | not evidence for every random generator or spatial process |
| clustered | equal allocation to four separated square clusters | behavior under uneven local density and large empty regions | not evidence for arbitrary cluster counts, separation, or density ratios |
| jittered-grid | one point per regular grid cell with bounded integer jitter, then deterministic shuffle | behavior on strongly structured, near-lattice inputs without exact grid degeneracy | deliberately does not test an exactly cocircular lattice or duplicate sites |
| near-collinear | distinct, evenly spaced x values with y confined to a thin integer band and a guaranteed nonzero cross product | behavior on a valid high-aspect-ratio thin strip | deliberately does not define behavior for exactly collinear input |

Every generator contract test asserts exact count, unique points, coordinate bounds, stable exact-input bytes across independent invocations, and successful geometry validation. The jittered-grid family is checked for uniqueness rather than relying on its construction alone.

For each family, distribution-summary.csv reports two different summaries: the three seed-level case medians, which are the input-level uncertainty sample; and all 15 successful timing repetitions pooled separately for runtime-noise inspection.

The dedicated distribution chart uses only the three seed-level case medians. It shows their median and MAD, p10–p90, and min–max, and labels the site count, seed count, repetitions per input, and successful sample count. Its p10–p90 is a descriptive interpolation across only those three seed-case medians—not a confidence interval or proof of universal robustness.

## Run Bundle Contract

Each `benchmarks/runs/<run-id>/` directory is a schema-versioned evidence bundle:

| Path | Contents |
| --- | --- |
| `manifest.json` | repository SHA and dirty state, exact runner command, profile, binary hash, metric definitions, and status counts |
| `environment.json` | OS/kernel, CPU, memory, compiler, Python, GNU time, locale, and relevant environment variables |
| `runs.jsonl` | one structured record per repetition; timing and memory fields are always numeric or `null` |
| `inputs/*.json` | the exact generated point set, deduplicated by SHA-256 |
| `logs/*.stdout.json` | unmodified machine-readable driver output, including the exact points |
| `logs/*.stderr.txt` | unmodified standard error |
| `logs/*.time.txt` | unmodified GNU time measurements |
| `summary.json` / `summary.csv` | successful-run counts plus median, median absolute deviation, p10/p90, and range |
| `validation.json` | input/output stability and per-case success checks |
| `charts/phase-medians.svg` | dependency-free deterministic chart generated from `summary.json` |
| distribution-summary.csv | per-distribution sample counts plus seed-level and separately pooled median/MAD/p10/p90 summaries |
| charts/distribution-comparison.svg | seed-level distribution comparison with MAD, p10/p90, range, and explicit sample labels |
| `checksums.sha256` | SHA-256 for every other file in the bundle |

Statuses are explicit: `ok`, `timeout`, `oom`, `error`, `skipped`, or `validation_failed`. A status is never placed in a numeric column. Timeouts terminate the entire child process group, so an over-budget geometry process is not left running in the background.

Bundles created under `benchmarks/runs/` are untracked scratch evidence until they have been reviewed. When a bundle is selected to support a published claim, commit the complete directory—including its manifest, environment, structured records, exact inputs, raw logs, validation, summaries, chart, and checksums—or copy it unchanged into a tracked `benchmarks/published/` directory and validate it again. A summary or chart without its raw bundle is not publication evidence.

Because bundle creation changes the worktree after provenance is captured, begin each publication run from a clean committed tree. Commit the finished evidence only after the run completes.

The Make targets always rebuild the benchmark entry object so embedded provenance cannot silently lag behind Git. Compiler contract version 2 embeds the compiler command, the compiler's version line, and the exact optimization/warning/language flags in the binary. Before creating a bundle, the runner executes the supplied compiler's version probe and requires those three values—plus commit, dirty state, and build profile—to match the binary. The validator repeats that comparison for the preflight record and every run; it also rejects a run or exact-input distribution, seed, count, or coordinate bound that disagrees with its manifest case. A stale binary or text-only compiler assertion is rejected.

## Publication Checklist

Before citing a result:

1. run from a clean, committed source tree
2. validate the bundle and its checksums
3. confirm all planned repetitions are `ok`
4. cite the repository commit, bundle path, distribution/generator/seed, exact input SHA-256, machine, repetitions, and named metric
5. report a robust center and spread rather than selecting the fastest repetition

Smoke bundles are useful implementation evidence, not headline performance evidence. Use the headline profile before publishing a 1,500-site timing claim.
