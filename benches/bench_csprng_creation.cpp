#include "bench_utils.hpp"
#include "randomshake/randomshake.hpp"
#include <array>
#include <benchmark/benchmark.h>

static void
bench_deterministic_csprng_creation(benchmark::State& state)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  for (auto _ : state) {
    benchmark::DoNotOptimize(seed);
    randomshake::randomshake_t csprng(seed);

    benchmark::DoNotOptimize(&csprng);
    benchmark::ClobberMemory();
  }
}

static void
bench_nondeterministic_csprng_creation(benchmark::State& state)
{
  for (auto _ : state) {
    randomshake::randomshake_t csprng;

    benchmark::DoNotOptimize(&csprng);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(bench_deterministic_csprng_creation)
  ->Name("deterministic_csprng/create")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);

BENCHMARK(bench_nondeterministic_csprng_creation)
  ->Name("non-deterministic_csprng/create")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
