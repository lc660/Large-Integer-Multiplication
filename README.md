# Integer Multiplication Benchmark

This project compares two decimal-string multiplication implementations:

- `quadratic`: grade-school multiplication, approximately `2n^2` digit operations.
- `fft`: convolution-based multiplication using an iterative Cooley-Tukey FFT.

The benchmark doubles input length from 1 digit until either method takes more than
the configured time limit, or `MAX_DIGITS` is reached.

## Build and Run

```bash
./scripts/run_benchmark.sh aws
```

Useful environment variables:

```bash
MAX_DIGITS=1048576 TIME_LIMIT_SECONDS=600 SEED=18647 ./scripts/run_benchmark.sh aws
```

The output CSV is written to `results/<machine>_multiply_benchmark.csv` with:

```text
digits,method,seconds,ops,flops_per_second,product_digits
```

## Operation Counts

For the quadratic method, each digit pair contributes one multiplication and one
addition, so the approximate operation count is:

```text
2n^2
```

For the FFT method, the implementation performs two forward FFTs and one inverse
FFT. Assuming one FFT costs `5N log2 N` operations and adding a small linear term
for pointwise multiplication and carry handling:

```text
15N log2 N + 6N
```

where `N` is the next power of two at least `2n`.
