import os
import subprocess
import argparse
from pathlib import Path
import sys


def compile_cpp(source_file, output_file="program", compiler_flags=None) -> Path | None:
    if compiler_flags is None:
        compiler_flags = []
    cmd = ["g++", source_file, "-o", output_file, "-fopenmp"] + compiler_flags

    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Ошибка компиляции: {result.stderr}")
        return None

    return Path(output_file)


def run_hyperfine(executable: Path, output_json, sizes, is_multithread):
    sizes_str = ",".join(map(str, sizes))
    threads_str = ",".join(map(str, range(1, os.cpu_count() + 1, 3)))  # type: ignore

    cmd = [
        "hyperfine",
        "--export-json",
        output_json,
        "--show-output",
        "-L",
        "size",
        sizes_str,
    ]
    if is_multithread:
        cmd.extend(
            [
                "-L",
                "threads",
                threads_str,
                f"./{executable.relative_to(Path('.'))} {{size}} {{threads}}",
            ]
        )
    else:
        cmd.append(f"./{executable.relative_to(Path('.'))} {{size}}")

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Ошибка hyperfine: {result.stderr}")
        print(f"Вывод: {result.stdout}")


def run_bench(source_file, executable_name, sizes, json_out_name, multithread_flag):
    if not Path(source_file).exists():
        print(f"Ошибка: файл {source_file} не существует")
        sys.exit(1)

    executable = compile_cpp(source_file, executable_name)
    if executable is None:
        print("Какие-то проблемы с компиляцией")
        return

    run_hyperfine(executable, json_out_name, sizes, multithread_flag)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Измерение производительности C++ программ"
    )
    parser.add_argument("source_file", help="C++ исходный файл")
    parser.add_argument("sizes", help="Размеры матриц", nargs=3)
    parser.add_argument("json_out")
    parser.add_argument("executable_name")
    parser.add_argument("multithread", type=bool)
    args = parser.parse_args()
    run_bench(
        args.source_file,
        args.executable_name,
        args.json_out,
        args.sizes,
        args.multithread,
    )
