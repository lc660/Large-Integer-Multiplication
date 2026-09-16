#!/usr/bin/env bash
set -euo pipefail

machine_name="${1:-$(hostname)}"
max_digits="${MAX_DIGITS:-1048576}"
time_limit="${TIME_LIMIT_SECONDS:-600}"
seed="${SEED:-18647}"

mkdir -p results
./scripts/build.sh

output="results/${machine_name}_multiply_benchmark.csv"
./build/int_multiply_bench \
  --max-digits "${max_digits}" \
  --time-limit "${time_limit}" \
  --seed "${seed}" \
  > "${output}"

echo "${output}"
