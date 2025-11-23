#!/usr/bin/env python3
import argparse
from pathlib import Path
import make_measure
import make_graphs


def main(seq_source_name, parallel_source_name, result_dir="result"):
    result_dir_path = Path(result_dir)
    if not result_dir_path.exists():
        result_dir_path.mkdir()
    result_seq_json = result_dir / Path("seq_bench.json")
    result_parallel_json = result_dir / Path("parallel_bench.json")

    sizes = [100, 1000, 2000]

    make_measure.run_bench(
        seq_source_name,
        result_dir_path / Path("seq-prog"),
        sizes,
        result_seq_json,
        False,
    )
    make_measure.run_bench(
        parallel_source_name,
        result_dir_path / Path("parallel-prog"),
        sizes,
        result_parallel_json,
        True,
    )

    graphs_path = result_dir_path / Path("graphs")
    if not graphs_path.exists():
        graphs_path.mkdir()

    make_graphs.make_graph(result_parallel_json, result_seq_json, graphs_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Замеры и построение графиков")
    parser.add_argument("seq_source_name", default="labA_07.cpp")
    parser.add_argument(
        "parallel_source_name", default="optimizes-versions/labA_07_collapse.cpp"
    )
    parser.add_argument("--result-dir")

    args = parser.parse_args()

    main(args.seq_source_name, args.parallel_source_name)
