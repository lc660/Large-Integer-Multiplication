#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using Complex = std::complex<double>;

constexpr double PI = 3.141592653589793238462643383279502884;

std::string random_decimal_string(std::size_t n, std::mt19937_64 &rng) {
    std::uniform_int_distribution<int> first_digit(1, 9);
    std::uniform_int_distribution<int> digit(0, 9);
    std::string s(n, '0');
    s[0] = static_cast<char>('0' + first_digit(rng));
    for (std::size_t i = 1; i < n; ++i) {
        s[i] = static_cast<char>('0' + digit(rng));
    }
    return s;
}

std::string normalize_digits(std::vector<int> digits) {
    int carry = 0;
    for (std::size_t i = 0; i < digits.size(); ++i) {
        int value = digits[i] + carry;
        digits[i] = value % 10;
        carry = value / 10;
    }
    while (carry > 0) {
        digits.push_back(carry % 10);
        carry /= 10;
    }
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }

    std::string out;
    out.reserve(digits.size());
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        out.push_back(static_cast<char>('0' + *it));
    }
    return out;
}

std::string multiply_quadratic(const std::string &a, const std::string &b) {
    if (a == "0" || b == "0") {
        return "0";
    }
    std::vector<int> digits(a.size() + b.size(), 0);
    for (std::size_t i = 0; i < a.size(); ++i) {
        int da = a[a.size() - 1 - i] - '0';
        for (std::size_t j = 0; j < b.size(); ++j) {
            int db = b[b.size() - 1 - j] - '0';
            digits[i + j] += da * db;
        }
    }
    return normalize_digits(std::move(digits));
}

void fft(std::vector<Complex> &a, bool invert) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }

    for (std::size_t len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * PI / static_cast<double>(len) * (invert ? -1.0 : 1.0);
        Complex wlen(std::cos(angle), std::sin(angle));
        for (std::size_t i = 0; i < n; i += len) {
            Complex w(1.0, 0.0);
            for (std::size_t j = 0; j < len / 2; ++j) {
                Complex u = a[i + j];
                Complex v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert) {
        for (Complex &x : a) {
            x /= static_cast<double>(n);
        }
    }
}

std::string multiply_fft(const std::string &a, const std::string &b) {
    if (a == "0" || b == "0") {
        return "0";
    }

    std::size_t n = 1;
    while (n < a.size() + b.size()) {
        n <<= 1;
    }

    std::vector<Complex> fa(n), fb(n);
    for (std::size_t i = 0; i < a.size(); ++i) {
        fa[i] = static_cast<double>(a[a.size() - 1 - i] - '0');
    }
    for (std::size_t i = 0; i < b.size(); ++i) {
        fb[i] = static_cast<double>(b[b.size() - 1 - i] - '0');
    }

    fft(fa, false);
    fft(fb, false);
    for (std::size_t i = 0; i < n; ++i) {
        fa[i] *= fb[i];
    }
    fft(fa, true);

    std::vector<int> digits(n);
    for (std::size_t i = 0; i < n; ++i) {
        digits[i] = static_cast<int>(std::llround(fa[i].real()));
    }
    return normalize_digits(std::move(digits));
}

double seconds_since(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double>(end - start).count();
}

std::uint64_t quadratic_ops(std::size_t n) {
    return 2ULL * static_cast<std::uint64_t>(n) * static_cast<std::uint64_t>(n);
}

double fft_ops(std::size_t digits) {
    std::size_t n = 1;
    while (n < 2 * digits) {
        n <<= 1;
    }
    return 15.0 * static_cast<double>(n) * std::log2(static_cast<double>(n)) +
           6.0 * static_cast<double>(n);
}

struct Args {
    std::size_t max_digits = 1 << 20;
    double time_limit_seconds = 600.0;
    std::uint64_t seed = 18647;
    bool verify = true;
};

Args parse_args(int argc, char **argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto require_value = [&](const std::string &name) -> std::string {
            if (i + 1 >= argc) {
                throw std::invalid_argument("missing value for " + name);
            }
            return argv[++i];
        };
        if (arg == "--max-digits") {
            args.max_digits = static_cast<std::size_t>(std::stoull(require_value(arg)));
        } else if (arg == "--time-limit") {
            args.time_limit_seconds = std::stod(require_value(arg));
        } else if (arg == "--seed") {
            args.seed = std::stoull(require_value(arg));
        } else if (arg == "--no-verify") {
            args.verify = false;
        } else {
            throw std::invalid_argument("unknown argument: " + arg);
        }
    }
    return args;
}

int main(int argc, char **argv) {
    try {
        Args args = parse_args(argc, argv);
        std::mt19937_64 rng(args.seed);

        std::cout << "digits,method,seconds,ops,flops_per_second,product_digits\n";
        std::cout << std::setprecision(10);

        for (std::size_t digits = 1; digits <= args.max_digits; digits <<= 1) {
            std::string a = random_decimal_string(digits, rng);
            std::string b = random_decimal_string(digits, rng);

            auto q_start = Clock::now();
            std::string q_product = multiply_quadratic(a, b);
            auto q_end = Clock::now();
            double q_seconds = seconds_since(q_start, q_end);
            double q_ops = static_cast<double>(quadratic_ops(digits));
            std::cout << digits << ",quadratic," << q_seconds << "," << q_ops << ","
                      << (q_ops / q_seconds) << "," << q_product.size() << "\n";
            std::cout.flush();

            auto f_start = Clock::now();
            std::string f_product = multiply_fft(a, b);
            auto f_end = Clock::now();
            double f_seconds = seconds_since(f_start, f_end);
            double f_ops_value = fft_ops(digits);
            std::cout << digits << ",fft," << f_seconds << "," << f_ops_value << ","
                      << (f_ops_value / f_seconds) << "," << f_product.size() << "\n";
            std::cout.flush();

            if (args.verify && q_product != f_product) {
                std::cerr << "mismatch at digits=" << digits << "\n";
                return 2;
            }
            if (q_seconds > args.time_limit_seconds || f_seconds > args.time_limit_seconds) {
                break;
            }
        }
    } catch (const std::exception &ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
