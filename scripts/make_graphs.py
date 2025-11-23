import argparse
import json
import os
from pathlib import Path
import sys
from matplotlib import ticker
import numpy as np
import matplotlib.pyplot as plt


def parse_results(outup_file):
    result_json_path = Path(outup_file)
    if not result_json_path.exists():
        print(f"Нет файла {result_json_path.name}")
        return
    with result_json_path.open() as f:
        result_json = json.load(f)

    means = {}

    for result in result_json["results"]:
        if result["parameters"]["size"] not in means:
            means[result["parameters"]["size"]] = [result["mean"]]
        else:
            means[result["parameters"]["size"]].append(result["mean"])

    return means


def draw_graphs_at_one(parallel_results, non_parallel, graphs_dir):
    processors = np.array(range(1, os.cpu_count() + 1, 3))  # type: ignore
    ideal_speedup = processors
    plt.style.use("seaborn-v0_8")
    _, ax = plt.subplots(figsize=(12, 7))

    ax.xaxis.set_major_locator(ticker.MultipleLocator(base=1))
    ax.yaxis.set_major_locator(ticker.MultipleLocator(base=1))

    ax.plot(
        processors,
        ideal_speedup,
        marker="o",
        markersize=6,
        linewidth=2.5,
        label="Идеальное ускорение",
        color="green",
        linestyle="--",
    )
    for size in parallel_results:
        ax.plot(
            processors,
            list(map(lambda p: non_parallel[size][0] / p, parallel_results[size])),
            marker="s",
            markersize=6,
            linewidth=2.5,
            label=f"ускорение для размера {size}",
        )

    ax.set_xlabel("Количество процессоров (ядер)", fontsize=12)
    ax.set_ylabel("Ускорение", fontsize=12)

    ax.legend(loc="upper left", fontsize=10)

    plt.tight_layout()
    plt.savefig(graphs_dir / Path("scale_ability_all.png"))


def make_graph(json_data_parallel, json_data_seq, graphs_dir):
    results = parse_results(json_data_parallel)
    if results is None:
        print("Результатов нет!")
        sys.exit(1)

    seq_res = parse_results(json_data_seq)
    if seq_res is None:
        print("Результатов последовательной версии нет!")
        sys.exit(1)

    draw_graphs_at_one(results, seq_res, graphs_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Построение графиков")
    parser.add_argument("json_data_parallel", help="Результаты замеров в json формате")
    parser.add_argument("json_data_seq", help="Результаты замеров в json формате")
    args = parser.parse_args()
    make_graph(args.json_data_parallel, args.json_data_seq, Path("graphs"))
