import os
import subprocess
import argparse
from pathlib import Path
import sys


def compile_cpp(
    source_file, output_file="program", compiler_flags=None, is_mpi=False
) -> Path | None:
    if compiler_flags is None:
        compiler_flags = []
    if not is_mpi:
        cmd = ["g++", source_file, "-o", output_file, "-fopenmp"] + compiler_flags
    else:
        cmd = ["mpic++", source_file, "-o", output_file] + compiler_flags

    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Ошибка компиляции: {result.stderr}")
        return None
    print(f"compiled {source_file}")
    return Path(output_file)


def run_hyperfine(executable: Path, output_json, sizes, is_multithread, is_mpi=False):
    sizes_str = ",".join(map(str, sizes))
    threads_str = ",".join(map(str, range(1, os.cpu_count() + 1, 3)))  # type: ignore

    print(f"run hyperfine at {executable}")
    if not is_mpi:
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
    else:
        cmd = [
            "hyperfine",
            "--export-json",
            output_json,
            "--show-output",
            "-L",
            "size",
            sizes_str,
            "-L",
            "threads",
            threads_str,
            f"mpirun -np {{threads}} ./{executable.relative_to(Path('.'))} {{size}}",
        ]

        # cmd.append(f"./{executable.relative_to(Path('.'))} {{size}}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Ошибка hyperfine: {result.stderr}")
        print(f"Вывод: {result.stdout}")
    print(f"finish hyperfine at {executable}")


def run_bench(
    source_file, executable_name, sizes, json_out_name, multithread_flag, is_mpi=False
):
    if not Path(source_file).exists():
        print(f"Ошибка: файл {source_file} не существует")
        sys.exit(1)

    executable = compile_cpp(source_file, executable_name, None, is_mpi)
    if executable is None:
        print("Какие-то проблемы с компиляцией")
        return

    run_hyperfine(executable, json_out_name, sizes, multithread_flag, is_mpi)


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
