# RandomSHAKE

(Turbo)SHAKE256 XOF -based Portable C++20 Cryptographically Secure Pseudo-Random Number Generator (CSPRNG) - plug and play with C++ standard library's `<random>` header's statistical distributions, for generating pseudo-random numbers.

## Introduction

C++11 introduced `<random>` header to its standard library, which offers multiple pseudo-random bit generator engines and statistical distributions. The design of the `<random>` header is very much modular, it's possible to plug any psuedo-random bit generator engine with any distribution and start squeezing results as per the rules of the statistical distribution. All that is needed, is providing the distribution with a source *U*niform *R*andom *B*it *G*enerator (URBG). One thing that the standard library's `<random>` header lacks is a cryptographically secure psuedo-random number generator engine. Using any of the provided engines such as Mersenne Twister or Linear Congruential engine with provided distributions, in cryptographic settings, might be quite catastrophic!

This is where "RandomSHAKE" comes, collecting inspiration from <https://seth.rocks/articles/cpprandom>.

"RandomSHAKE" is a *C*ryptographically *S*ecure *P*seudo-*R*andom *N*umber *G*enerator (CSPRNG) engine, which is backed by (Turbo)SHAKE256 eXtendable Output Function (XOF) with occasional ratcheting. It's optional to specify which XOF you want to use. By default you get TurboSHAKE256, which doubles the throughput compared to SHAKE256. In case you really want SHAKE256, you need to be explicit. For initializing the CSPRNG, either you can rely on the convenient default constructor, which samples initial seed from the **non-deterministic** `std::random_device` API or you can explicitly supply a seed for **deterministic** and reproducible behaviour. It's very easy to plug this CSPRNG engine into any of the C++ standard library's statistical distributions defined in `<random>` header. Just plug and play. And now you can use those distributions with "RandomSHAKE" CSPRNG, in cryptographic settings - producing integers or floats, whatever you need. It also offers API for generating arbitrary long byte stream at a time.

> [!CAUTION]
> Using the non-deterministic CSPRNG initialization API is very convenient, but there is a caveat - this CSPRNG samples its seed from `std::random_device` engine, which is supposed to be non-deterministic, but is not guaranteed to be - it's implementation-defined behavior. I strongly advise you to read <https://en.cppreference.com/w/cpp/numeric/random/random_device>.

```cpp
// Simply declare CSPRNG, producing pseudo-random uint8_t, backed by TurboSHAKE256 XOF.
randomshake::randomshake_t csprng;

// --- Or, declare alias for shorter type name. ---

// Result type: uint8_t, XOF: TurboSHAKE256. Default case.
using csprng_t = randomshake::randomshake_t<>;

// Or
// Result type: uint16_t, XOF: TurboSHAKE256. Override only default result data type.
using csprng_t = randomshake::randomshake_t<uint16_t>;

// Or
// Result type: uint8_t, XOF: SHAKE256. Override only default XOF.
using csprng_t = randomshake::randomshake_t<uint8_t, randomshake::xof_kind_t::SHAKE256>;

// Or
// Result type: uint64_t, XOF: SHAKE256. Override both default result data type and XOF.
using csprng_t = randomshake::randomshake_t<uint64_t, randomshake::xof_kind_t::SHAKE256>;

csprng_t csprng; // Default constructor. Automatically seeded using `std::random_device`. Non-deterministic.
```

While for the deterministic CSPRNG, as an user, it's your responsibility to supply a seed of required byte length with sufficient entropy. You should use this CSPRNG API, if you need reproducible random bytes.

```cpp
std::array<uint8_t, csprng_t::seed_byte_len> seed{};
seed.fill(0xde);       // Please don't do it in any practical scenario !

csprng_t csprng(seed); // Explicit constructor. Initialized by the seed, we supply. Deterministic.
```

Using the CSPRNG instance for sampling random value(s), is super easy.

```cpp
// Sample a single random uint8_t or uint16_t or uint32_t or uint64_t. Based on result data type template parameter.
const auto random_value = csprng();

// or
// Fill the vector, sampling one byte at a time.
std::vector<uint8_t> rand_values(16, 0x00);
std::ranges::generate(rand_values, [&]() { return csprng(); });

// or
// Fill the vector, by squeezing many bytes at a time. Convenient `generate` API for squeezing arbitrary many bytes.
csprng.generate(rand_values);
```

### "RandomSHAKE" CSPRNG Performance Overview

CSPRNG Operation | Time taken/ Throughput achieved on AWS EC2 Instance `c8i.large` | Time taken/ Throughput achieved on AWS EC2 Instance `c8g.large`
--- | --- | ---
Deterministic seeding of instance | 1.3 us | 1.8 us
Non-Deterministic seeding of instance | 38.5 us | 11.5 us
--- | --- | ---
Sampling of `u8` | 625.4 MB/s | 255.2 MB/s
Sampling of `u16` | 730.3 MB/s | 363.8 MB/s
Sampling of `u32` | 797.1 MB/s | 468.1 MB/s
Sampling of `u64` | 833.3 MB/s | 545.8 MB/s
Sampling arbitrary long byte sequence (using TurboSHAKE256 XOF, default) | 869.2 MB/s | 637.5 MB/s
Sampling arbitrary long byte sequence (using SHAKE256 XOF, explicit) | 485.2 MB/s | 354.7 MB/s

## Prerequisites

- A C++ compiler such as `g++`/ `clang++`, with support for C++20 standard library.
- CMake (>= 3.28).
- `google-test` and `google-benchmark` can either be installed system-wide, or fetched automatically by CMake (pass `-DRANDOMSHAKE_FETCH_DEPS=ON`).

> [!NOTE]
> If you are on a machine running GNU/Linux kernel and you want to obtain CPU cycles or Cycles/ byte or instruction/ cycle etc., when benchmarking RandomSHAKE CSPRNG, you should consider building `google-benchmark` library yourself with `libPFM` support, following the step-by-step guide @ <https://gist.github.com/itzmeanjan/05dc3e946f635d00c5e0b21aae6203a7>. Find more about libPFM @ <https://perfmon2.sourceforge.net>.

> [!NOTE]
> I use the BASH script @ <https://gist.github.com/itzmeanjan/84b7df57604708e33f04fc43e55ecb0c> to quickly setup a GNU/Linux machine, so that I can run tests and benchmarks. Running this script does the whole setup phase for you on Ubuntu and adjacent family of Linux distributions.

## Testing

For running all functional correctness tests, issue following commands.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRANDOMSHAKE_BUILD_TESTS=ON -DRANDOMSHAKE_FETCH_DEPS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

To enable sanitizers or specify a compiler:

```bash
# With AddressSanitizer
cmake -B build -DRANDOMSHAKE_BUILD_TESTS=ON -DRANDOMSHAKE_FETCH_DEPS=ON -DRANDOMSHAKE_ASAN=ON -DCMAKE_BUILD_TYPE=Release # Should be run in Debug too

# With UndefinedBehaviorSanitizer
cmake -B build -DRANDOMSHAKE_BUILD_TESTS=ON -DRANDOMSHAKE_FETCH_DEPS=ON -DRANDOMSHAKE_UBSAN=ON -DCMAKE_BUILD_TYPE=Release

# Specify compiler
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRANDOMSHAKE_BUILD_TESTS=ON -DRANDOMSHAKE_FETCH_DEPS=ON -DCMAKE_CXX_COMPILER=clang++
```

## Benchmarking

For benchmarking both the creation of a CSPRNG instance and sampling of random unsigned integers from it, issue following commands.

> [!CAUTION]
> You must disable CPU frequency scaling during benchmarking, following [this](https://github.com/google/benchmark/blob/4931aefb51d1e5872b096a97f43e13fa0fc33c8c/docs/reducing_variance.md) guide.

```bash
cmake -B build -DRANDOMSHAKE_BUILD_BENCHMARKS=ON -DRANDOMSHAKE_FETCH_DEPS=ON -DCMAKE_BUILD_TYPE=Release -DRANDOMSHAKE_NATIVE_OPT=ON
cmake --build build -j

# Run benchmarks, with cpu-cycle count. Must be built with libPFM.
./build/randomshake_benchmarks --benchmark_min_warmup_time=.1 --benchmark_enable_random_interleaving=false --benchmark_repetitions=10 --benchmark_min_time=0.1s --benchmark_display_aggregates_only=true --benchmark_counters_tabular=true --benchmark_perf_counters=CYCLES

# Run benchmarks, with just time.
./build/randomshake_benchmarks --benchmark_min_warmup_time=.1 --benchmark_enable_random_interleaving=false --benchmark_repetitions=10 --benchmark_min_time=0.1s --benchmark_display_aggregates_only=true --benchmark_counters_tabular=true
```

> [!NOTE]
> If `libpfm` is installed on the system, it will be automatically linked to the benchmark binary, enabling hardware performance counter support.

I've run benchmarks on some platforms and here are the results.

### Benchmarking on DESKTOP-grade Machine(s)

- x86_64. 12th Gen Intel(R) Core(TM) i7-1260P. JSON dump @ [bench_result_on_Linux_6.17.0-6-generic_x86_64_with_g++_15](./bench_result_on_Linux_6.17.0-6-generic_x86_64_with_g++_15.json).

### Benchmarking on SERVER-grade Machine(s)

- x86_64. AWS EC2 Instance `c8i.large` i.e. Intel Xeon 6975P-C. JSON dump @ [bench_result_on_Linux_6.14.0-1015-aws_x86_64_with_g++_13](./bench_result_on_Linux_6.14.0-1015-aws_x86_64_with_g++_13.json).
- aarch64. AWS EC2 Instance `c8g.large` i.e. AWS Graviton4. JSON dump @ [bench_result_on_Linux_6.14.0-1015-aws_aarch64_with_g++_13](./bench_result_on_Linux_6.14.0-1015-aws_aarch64_with_g++_13.json).

## Development

### Static Analysis

Run `clang-tidy` across all headers via the example translation units:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRANDOMSHAKE_BUILD_EXAMPLES=ON
cmake --build build --target tidy
```

### Code Formatting

Run `clang-format` on all source files (requires a `.clang-format` file in the project root):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRANDOMSHAKE_BUILD_EXAMPLES=ON -DRANDOMSHAKE_BUILD_TESTS=ON
cmake --build build --target format
```

## Usage

`randomshake` is a header-only C++20 library, designed to be easy to integrate into your CMake project.

### Integration

You can install `randomshake` system-wide and use `find_package`:

```bash
# Install system-wide (default prefix: /usr/local)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build

# Install to custom directory (e.g., ./dist)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./dist
cmake --build build
cmake --install build
```

Or using `FetchContent` in your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
  randomshake
  GIT_REPOSITORY https://github.com/itzmeanjan/RandomShake.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(randomshake)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE randomshake)
```

Alternatively, just point your compiler's include path to the `./include` directory.

See [examples/CMakeLists.txt](./examples/CMakeLists.txt) for a standalone consumption example that uses `find_package` with `FetchContent` fallback.

### Examples

We maintain a few examples of using "RandomSHAKE" API with various C++ STL distributions @ [examples](./examples) directory. Build and run them by issuing:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DRANDOMSHAKE_BUILD_EXAMPLES=ON
cmake --build build -j
```

```bash
$ ./build/csprng_with_binomial_dist_example
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 515
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 522
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 499
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 500
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 526
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 513
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 509
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 503
[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: 491
```

> [!NOTE]
> In above demonstration, I'm showing how to use "RandomSHAKE" CSPRNG with C++ standard library's Binomial Distribution, but it should be fairly easy, plugging this CSPRNG with any other available distribution in `<random>` header.

In case you just want to generate arbitrary many random bytes, there is an API `generate` - which can generate arbitrary many random bytes and it should be fine calling this as many times needed. Ratcheting is taken care of under the hood.
