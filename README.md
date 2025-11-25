# RandomSHAKE

(Turbo)SHAKE256 XOF -based Portable C++20 Cryptographically Secure Pseudo Random Number Generator (CSPRNG) - plug and play with C++ standard library's `<random>` header's statistical distributions.

## Why ?

C++11 introduced `<random>` header to its standard library, which offers multiple pseudo-random number generator engines and statistical distributions. The design of the `<random>` header is very much modular, it's possible to plug any psuedo-random number generator engine with any distribution and start getting results as per the rules of the statistical distribution. All that is needed, is providing the distribution with a source *U*niform *R*andom *N*umber *G*enerator (URNG). One thing that the standard library's `<random>` header lacks is a cryptographically secure psuedo-random number generator engine. Using any of the provided engines such as Mersenne Twister or Linear Congruential engine with provided distributions, in cryptographic settings, might be quite catastrophic!

This is where "RandomSHAKE" comes, collecting inspiration from <https://seth.rocks/articles/cpprandom>.

"RandomSHAKE" is a *C*ryptographically *S*ecure *P*seudo-*R*andom *N*umber *G*enerator (CSPRNG) engine, which is backed by (Turbo)SHAKE256 eXtendable Output Function (XOF) with occasional ratcheting, generating unsigned integer (of all standard bit-widths) stream. It's optional to specify which XOF you want to use. By default you get TurboSHAKE256, which doubles the throughput compared to SHAKE256. In case you really want SHAKE256, you need to be explicit. For initializing the CSPRNG, either you can rely on the convenient default constructor, which samples initial seed from the **non-deterministic** `std::random_device` API or you can explicitly supply a seed for **deterministic** and reproducible behaviour. It's very easy to plug this CSPRNG engine into any of the C++ standard library's statistical distributions defined in `<random>` header. Just plug and play. And now you can use those distributions with "RandomSHAKE" CSPRNG, in cryptographic settings - producing integers or floats, whatever you need. It also offers API for generating arbitrary long byte stream at a time.

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

## How to setup ?

- Ensure that you have access to a C++ compiler, which supports compiling C++20 program.

```bash
# I'm using
$ c++ --version
c++ (Ubuntu 15.2.0-4ubuntu4) 15.2.0
```

- You will also need both `make` and `cmake` for building this project and test framework/ benchmark harness.

```bash
$ make --version
GNU Make 4.4.1

$ cmake --version
cmake version 3.31.6
```

- For running tests, you must have `google-test` globally installed. Follow steps described @ <https://github.com/google/googletest/blob/main/googletest/README.md>.
- For running benchmarks, you must have `google-benchmark` globally installed. You may find steps described @ <https://github.com/google/benchmark/?tab=readme-ov-file#installation> helpful.

> [!NOTE]
> In case you are planning to run benchmarks on a machine which runs GNU/Linux kernel, I suggest you build `google-benchmark` with libPFM, so that you get to know how many CPU cycles does it take for each benchmarked functions to execute. I describe the steps @ <https://gist.github.com/itzmeanjan/05dc3e946f635d00c5e0b21aae6203a7>.

> [!NOTE]
> I use the BASH script @ <https://gist.github.com/itzmeanjan/84b7df57604708e33f04fc43e55ecb0c> to quickly setup a GNU/Linux machine, so that I can run tests and benchmarks. Running this script does the whole setup phase for you on Ubuntu and adjacent family of Linux distributions.

## How to test ?

For running all functional correctness tests, just issue

```bash
make test -j
```

```bash
PASSED TESTS (9/9):
       1 ms: build/test/test.out RandomSHAKETestUtility.Bit_Flipping_Must_Work_Correctly
       4 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Oneshot_vs_Multishot_Squeezing
      11 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Using_Same_Seed_With_Diff_XOF_Kind_Produces_Ne_Output
      11 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Public_API
      11 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Detect_Ratchet_Working_For_TurboSHAKE256_XOF
      16 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Using_Diff_Seed_Produces_Ne_Output
      17 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Using_Same_Seed_Produces_Eq_Output
      18 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Detect_Ratchet_Working_For_SHAKE256_XOF
      20 ms: build/test/test.out RandomSHAKE.Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Result_Type_Produces_Eq_Output
```

> [!NOTE]
> There is a help menu, which introduces you to all available commands; just run `make` from the root directory of this project.

You may want to run tests with AddressSanitizer or UndefinedBehaviorSanitizer enabled, in either debug mode or release mode. You can simply issue

```bash
make debug_asan_test -j
make debug_ubsan_test -j

make release_asan_test -j
make release_ubsan_test -j
```

By default Make will use default c++ compiler of the system to build tests, but you specify your choice, by setting the `CXX` variable, before invoking any of the Make commands.

```bash
CXX=clang++ make test -j
```

## How to benchmark ?

For benchmarking both the creation of a CSPRNG instance and sampling of random unsigned integers from it, just issue.

```bash
make benchmark -j
```

In case, you decided to build google-benchmark with libPFM, so that you can get access to h/w CPU cycles counter, you have to issue

```bash
make perf -j
```

> [!CAUTION]
> You must disable CPU frequency scaling during benchmarking. I found guide @ <https://github.com/google/benchmark/blob/4931aefb51d1e5872b096a97f43e13fa0fc33c8c/docs/reducing_variance.md> helpful.

I've run benchmarks on some platforms and here are the results.

### Benchmarking on DESKTOP-grade Machine(s)

- x86_64. 12th Gen Intel(R) Core(TM) i7-1260P. JSON dump @ [bench_result_on_Linux_6.17.0-6-generic_x86_64_with_g++_15](./bench_result_on_Linux_6.17.0-6-generic_x86_64_with_g++_15.json).

### Benchmarking on SERVER-grade Machine(s)

- x86_64. AWS EC2 Instance `c8i.large` i.e. Intel Xeon 6975P-C. JSON dump @ [bench_result_on_Linux_6.14.0-1015-aws_x86_64_with_g++_13](./bench_result_on_Linux_6.14.0-1015-aws_x86_64_with_g++_13.json).
- aarch64. AWS EC2 Instance `c8g.large` i.e. AWS Graviton4. JSON dump @ [bench_result_on_Linux_6.14.0-1015-aws_aarch64_with_g++_13](./bench_result_on_Linux_6.14.0-1015-aws_aarch64_with_g++_13.json).

## How to use ?

Using "RandomSHAKE" CSPRNG is very easy.

- Clone this repository, whiile importing all git submodule -based dependencies.

```bash
git clone https://github.com/itzmeanjan/RandomShake.git --recurse-submodules
```

- Write a C++ function, using this CSPRNG, while including "randomshake/randomshake.hpp" header file.

```cpp
// csprng_with_binomial_dist.cpp

#include "randomshake/randomshake.hpp"
#include <cstdlib>
#include <random>
#include <ranges>

// This example collects inspiration from https://seth.rocks/articles/cpprandom. See the last code-snippet.
int
main()
{
  randomshake::randomshake_t csprng;
  std::binomial_distribution dist{ 1'000, .5 };

  for (auto _ : std::ranges::iota_view{ 1, 10 }) {
    std::cout << "[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: " << dist(csprng) << '\n';
  }

  return EXIT_SUCCESS;
}
```

> [!NOTE]
> In above demonstration, I'm showing how to use "RandomSHAKE" CSPRNG with C++ standard library's Binomial Distribution, but it should be fairly easy, plugging this CSPRNG with any other available distribution in `<random>` header.

- Compile the C++ translation unit, while including path to both "RandomSHAKE" and "sha3".

```bash
c++  -std=c++20 -Wall -Wextra -Wpedantic -O3 -march=native -I ./include -I ./sha3/include examples/csprng_with_binomial_dist.cpp
```

- And finally run the executable.

```bash
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

In case you just want to generate arbitrary many random bytes, there is an API `generate` - which can generate arbitrary many random bytes and it should be fine calling this as many times needed. Ratcheting is taken care of under the hood.

I maintain a few examples of using "RandomSHAKE" API with various C++ STL distributions @ [examples](./examples) directory. You can run them all by `$ make example -j`.
