import csv
import matplotlib.pyplot as plt
import sys

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

    data = [(l, c) for l, c in data if l <= 200]
    if not data:
        raise RuntimeError("No data <= 1000 ns")

    latencies, counts = zip(*data)
    buckets = {}
    for latency, count in zip(latencies, counts):
        buckets[latency] = buckets.get(latency, 0) + count

    bx = sorted(buckets.keys())
    by = [buckets[b] for b in bx]

    plt.figure(figsize=(10, 6))
    plt.bar(bx, by)

    plt.xlabel("Latency bucket (ns)")
    plt.ylabel("Count")
    plt.title("Latency Distribution (≤ 1000 ns)")

    text = (
        f"avg = {avg_latency:.2f} ns\n"
        f"p50 = {p50} ns\n"
        f"p95 = {p95} ns\n"
        f"p99 = {p99} ns\n"
        f"p999 = {p999} ns"
    )

    plt.text(
        0.98, 0.95,
        text,
        transform=plt.gca().transAxes,
        fontsize=10,
        verticalalignment="top",
        horizontalalignment="right",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.8)
    )

    plt.axvline(
        x=p50,
        linestyle="--",
        linewidth=2,
        color="red",
        alpha=0.8,
        label="p50"
    )

    plt.tight_layout()
    plt.savefig(outfile, dpi=300)
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Please specify the input dir and the output dir")
        sys.exit(1)

    indir = sys.argv[1];
    outdir = sys.argv[2];

    plot_latency_distribution(
        indir  + "parsing_and_order_book_latency_distribution.csv",
        outdir + "parsing_and_order_book_latency_distribution.png"
    )

    plot_latency_distribution(
        indir  + "parsing_latency_distribution.csv",
        outdir + "parsing_latency_distribution.png"
    )

