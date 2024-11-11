# RandomShake
Shake256 Xof -based Portable C++20 Cryptographically Secure Pseudo Random Number Generator (CSPRNG) - plug and play with C++ `<random>` header's random distributions.

## Why ?

C++11 introduced `<random>` header to its standard library, which offers multiple pseudo-random number generator engines and distributions. The design of the `<random>` header is very much modular, it's possible to plug any psuedo-random number generator engine with any distribution and start getting results as per the rules of the statistical distribution. All that is needed is providing the distribution with a source *u*niform *r*andom *n*umber *g*enerator (URNG). One thing that the standard library's `<random>` header lacks is the psuedo-random number generator engines, which come by default, are not cryptographically secure. Hence using those engines with provided distributions, in cryptographic settings, might be quite catastrophic !

This is where "RandomShake" comes, collecting inspiration from https://seth.rocks/articles/cpprandom.

"RandomShake" is a *C*ryptographically *S*ecure *P*seudo-*R*andom *N*umber *G*enerator (CSPRNG) engine, which is backed by Shake256 extendable output function (Xof) with frequent ratcheting of the underlying keccak-1600 permutation state, generating unsigned integer (of all standard bit-widths) stream for all three NIST security levels. It's very easy to plug this CSPRNG engine into any of the C++ standard library's random distributions. Just plug and play. And now you can use those distributions with "RandomShake" CSPRNG, in cryptographic settings. It also offers API for generating arbitrary many bytes at a time. "RandomShake" offers following two ways for seeding the internal Shake256 state.

- [**Non-deterministic CSPRNG**] Sampling k -bit non-deterministic randomness from `std::random_device` engine.
- [**Deterministic and re-producible CSPRNG**] Taking k -bit seed as input from function caller.

Using the non-deterministic CSPRNG is very convenient, but there is a caveat - this CSPRNG samples its seed from `std::random_device` engine, which is supposed to be non-deterministic, but is not guaranteed to be - it's implementation-defined behavior. I strongly advise you to read https://en.cppreference.com/w/cpp/numeric/random/random_device.

```cpp
using csprng_256b_t = randomshake::randomshake_t<256>;

csprng_256b_t csprng; // Automatically seeded using `std::random_device`
```

While for the deterministic CSPRNG, as an user, it's your responsibility to supply a seed of required byte length with sufficient entropy - this CSPRNG is the one to use, if you need reproducible random bytes.

```cpp
std::array<uint8_t, csprng_256b_t::seed_byte_len> seed{};
seed.fill(0xde);            // Please don't do it in any practical scenario !

csprng_256b_t csprng(seed); // Initialized by the seed we supply
```

## How to setup ?

- Ensure that you have access to a C++ compiler, which supports compiling C++20 program.

```bash
# I'm using
$ c++ --version
c++ (Ubuntu 14.2.0-4ubuntu2) 14.2.0
```

- You will also need both `make` and `cmake` for building this project and test framework/ benchmark harness.

```bash
$ make --version
GNU Make 4.3

$ cmake --version
cmake version 3.30.3
```

- For running tests, you must have `google-test` globally installed. Follow steps described @ https://github.com/google/googletest/blob/main/googletest/README.md.
- For running benchmarks, you must have `google-benchmark` globally installed. You may find steps described @ https://github.com/google/benchmark/?tab=readme-ov-file#installation helpful.

> [!NOTE]
> In case you are planning to run benchmarks on a machine which runs GNU/Linux kernel, I suggest you build `google-benchmark` with libPFM, so that you get to know how many CPU cycles does it take for each benchmarked functions to execute. I describe the steps @ https://gist.github.com/itzmeanjan/05dc3e946f635d00c5e0b21aae6203a7.

> [!NOTE]
> I use the BASH script @ https://gist.github.com/itzmeanjan/84b7df57604708e33f04fc43e55ecb0c to quickly setup a GNU/Linux machine, so that I can run tests and benchmarks. Running this script does the whole setup phase for you on Ubuntu and large family of OS.

## How to test ?

For running all functional correctness tests, just issue

```bash
make test -j
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
> You must disable CPU frequency scaling during benchmarking. I found guide @ https://github.com/google/benchmark/blob/4931aefb51d1e5872b096a97f43e13fa0fc33c8c/docs/reducing_variance.md helpful.

I've run benchmarks on some platforms and here are the results.

Benchmarking Results on DESKTOP -grade Machine(s)
---

### On 12th Gen Intel(R) Core(TM) i7-1260P

Compiled with `g++ (Ubuntu 14.2.0-4ubuntu2) 14.2.0`, while running on `Linux 6.11.0-9-generic x86_64`.

I maintain the benchmark results in JSON format @ [bench_result_on_Linux_6.11.0-9-generic_x86_64_with_g++_14](./bench_result_on_Linux_6.11.0-9-generic_x86_64_with_g++_14.json).

### On Raspberry Pi 4B i.e. ARM Cortex-A72

Compiled with `g++ (Debian 12.2.0-14) 12.2.0`, while running on `Linux 6.6.51+rpt-rpi-v8 aarch64`.

Find the benchmark results in JSON format @ [bench_result_on_Linux_6.6.51+rpt-rpi-v8_aarch64_with_g++_12](./bench_result_on_Linux_6.6.51+rpt-rpi-v8_aarch64_with_g++_12.json).

Benchmarking Results on SERVER -grade Machine(s)
---

### On Intel(R) Xeon(R) Platinum 8488C i.e. AWS EC2 Instance `c7i.large`

Compiled with `g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0`, while running on `Linux 6.8.0-1016-aws x86_64`.

Find the benchmark results in JSON format @ [bench_result_on_Linux_6.8.0-1016-aws_x86_64_with_g++_13](./bench_result_on_Linux_6.8.0-1016-aws_x86_64_with_g++_13.json).

### On AWS EC2 Instance `c8g.large` i.e. AWS Graviton4

Compiled with `g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0`, while running on `Linux 6.8.0-1016-aws aarch64`.

Find the benchmark results in JSON format @ [bench_result_on_Linux_6.8.0-1016-aws_aarch64_with_g++_13](./bench_result_on_Linux_6.8.0-1016-aws_aarch64_with_g++_13.json).

## How to use ?

Using "RandomShake" CSPRNG is very easy.

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
  randomshake::randomshake_t<256> csprng;
  std::binomial_distribution dist{ 1'000, .5 };

  for (auto _ : std::ranges::iota_view{ 1, 10 }) {
    std::cout << "[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: " << dist(csprng) << '\n';
  }

  return EXIT_SUCCESS;
}
```

> [!NOTE]
> In above demonstration, I'm showing how to use "RandomShake" CSPRNG (at 256 -bit security) with C++ standard library's Binomial Distribution, but it should be fairly easy, plugging this CSPRNG with any other available distribution.

- Compile the C++ translation unit, while including path to both "RandomShake" and "sha3".

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

In case you just want to generate arbitrary many random bytes, there is an API `generate` - which can generate arbitrary many random bytes and it should be fine calling this as many times needed.

I maintain a few examples of using "RandomShake" API with various C++ STL distributions @ [examples](./examples) directory.
