import csv
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
from pathlib import Path
import sys

MAX_LINEAR_PLOT_LATENCY_NS = 2_000
MAX_LOG_PLOT_LATENCY_NS = 10_000

def weighted_percentile(latencies, counts, percentile):
    total = sum(counts)
    threshold = total * percentile
    acc = 0
    for latency, count in zip(latencies, counts):
        acc += count
        if acc >= threshold:
            return latency
    return latencies[-1]

def plot_latency_distribution(infile, outfile):
    latencies = []
    counts = []

    with open(infile, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            latencies.append(int(row["latency_ns"]))
            counts.append(int(row["count"]))

    if not latencies:
        raise RuntimeError("CSV is empty")

    data = sorted(zip(latencies, counts), key=lambda x: x[0])
    latencies, counts = zip(*data)

    p50 = weighted_percentile(latencies, counts, 0.50)
    p95 = weighted_percentile(latencies, counts, 0.95)
    p99 = weighted_percentile(latencies, counts, 0.99)
    p999 = weighted_percentile(latencies, counts, 0.999)

    total_count = sum(counts)
    avg_latency = sum(l * c for l, c in zip(latencies, counts)) / total_count

    linear_plot_data = [
        (latency, count)
        for latency, count in zip(latencies, counts)
        if latency <= MAX_LINEAR_PLOT_LATENCY_NS
    ]
    if not linear_plot_data:
        raise RuntimeError(f"No data <= {MAX_LINEAR_PLOT_LATENCY_NS} ns")

    log_plot_data = [
        (latency, count)
        for latency, count in zip(latencies, counts)
        if latency <= MAX_LOG_PLOT_LATENCY_NS
    ]
    if not log_plot_data:
        raise RuntimeError(f"No data <= {MAX_LOG_PLOT_LATENCY_NS} ns")

    linear_buckets = {}
    for latency, count in linear_plot_data:
        linear_buckets[latency] = linear_buckets.get(latency, 0) + count

    log_buckets = {}
    for latency, count in log_plot_data:
        log_buckets[latency] = log_buckets.get(latency, 0) + count

    linear_bx = sorted(linear_buckets.keys())
    linear_by = [linear_buckets[b] for b in linear_bx]

    log_bx = sorted(log_buckets.keys())
    log_by = [log_buckets[b] for b in log_bx]
    log_by_normalized = [count / total_count for count in log_by]

    fig, (ax_linear, ax_log) = plt.subplots(
        2,
        1,
        figsize=(10, 9),
        gridspec_kw={"height_ratios": [2, 1]}
    )

    ax_linear.bar(linear_bx, linear_by)
    ax_linear.set_xlabel("Latency bucket (ns)")
    ax_linear.set_ylabel("Count")
    ax_linear.set_title(f"{Path(infile).stem} (<= {MAX_LINEAR_PLOT_LATENCY_NS} ns)")

    text = (
        f"avg = {avg_latency:.2f} ns\n"
        f"p50 = {p50} ns\n"
        f"p95 = {p95} ns\n"
        f"p99 = {p99} ns\n"
        f"p999 = {p999} ns"
    )

    ax_linear.text(
        0.98, 0.95,
        text,
        transform=ax_linear.transAxes,
        fontsize=10,
        verticalalignment="top",
        horizontalalignment="right",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.8)
    )

    ax_log.bar(log_bx, log_by_normalized)
    ax_log.set_yscale("log")
    ax_log.set_xlim(0, MAX_LOG_PLOT_LATENCY_NS)
    ax_log.set_xlabel("Latency bucket (ns)")
    ax_log.set_ylabel("Share of samples")
    ax_log.yaxis.set_major_formatter(mticker.PercentFormatter(xmax=1.0))
    ax_log.set_title(f"Normalized bucket share, log scale (<= {MAX_LOG_PLOT_LATENCY_NS} ns)")

    for ax in (ax_linear, ax_log):
        ax.axvline(
            x=p50,
            linestyle="--",
            linewidth=2,
            color="red",
            alpha=0.8,
            label="p50"
        )
        ax.legend()
        ax.grid(axis="y", alpha=0.25)

    fig.tight_layout()
    fig.savefig(outfile, dpi=300)
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Please specify the input dir and the output dir")
        sys.exit(1)

    indir = Path(sys.argv[1])
    outdir = Path(sys.argv[2])

    csv_files = sorted(indir.glob("*latency_distribution*.csv"))
    if not csv_files:
        raise RuntimeError(f"No latency distribution CSV files found in {indir}")

    outdir.mkdir(parents=True, exist_ok=True)

    for csv_file in csv_files:
        plot_latency_distribution(
            str(csv_file),
            str(outdir / f"{csv_file.stem}.png")
        )
