#!/usr/bin/env bash
set -euo pipefail

mkdir -p build
g++ -O3 -std=c++17 -march=native -Wall -Wextra -pedantic \
  src/int_multiply_bench.cpp \
  -o build/int_multiply_bench
