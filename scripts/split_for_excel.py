#!/usr/bin/env python3
import csv
import pathlib
import sys


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: split_for_excel.py results/<machine>_multiply_benchmark.csv", file=sys.stderr)
        return 1

    source = pathlib.Path(sys.argv[1])
    stem = source.stem.replace("_multiply_benchmark", "")
    output_dir = source.parent

    rows_by_method = {"quadratic": [], "fft": []}
    with source.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows_by_method[row["method"]].append({
                "machine": stem,
                "digits": row["digits"],
                "seconds": row["seconds"],
                "ops": row["ops"],
                "flops_per_second": row["flops_per_second"],
            })

    fieldnames = ["machine", "digits", "seconds", "ops", "flops_per_second"]
    for method, rows in rows_by_method.items():
        target = output_dir / f"{stem}_{method}_for_excel.csv"
        with target.open("w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)
        print(target)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
