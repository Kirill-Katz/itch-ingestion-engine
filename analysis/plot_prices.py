import csv
import matplotlib.pyplot as plt
import sys

def plot_prices(infile, outfile):
    prices = []

    with open(infile, newline="") as f:
        reader = csv.DictReader(f)

        skipped = 0
        for row in reader:
            # skipping the first 50 best bids, because at the start of the trading day
            # they start very low prices which breaks the plotting of the graph by introducing a
            # huge spike, after the first 50 ticks the bids normalize
            if skipped < 50:
                skipped += 1
                continue

            if int(row["price"]) != 0:
                prices.append(int(row["price"]) / 10_000)

    plt.figure()
    plt.plot(prices)
    plt.xlabel("Index")
    plt.ylabel("Price")
    plt.title("Price Distribution")
    plt.grid(True)

    plt.tight_layout()
    plt.savefig(outfile, dpi=300)
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("python plot_prices.py [prices csv] [path to output file]")
        sys.exit(1)

    infile = sys.argv[1];
    outfile = sys.argv[2];

    plot_prices(infile, outfile)
