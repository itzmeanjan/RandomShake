#include "bench_utils.hpp"
#include "randomshake/randomshake.hpp"
#include <array>
#include <benchmark/benchmark.h>

template<size_t bit_security_level>
static void
bench_deterministic_csprng_creation(benchmark::State& state)
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  for (auto _ : state) {
    benchmark::DoNotOptimize(seed);
    randomshake::randomshake_t<bit_security_level> csprng(seed);

    benchmark::DoNotOptimize(&csprng);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(bench_deterministic_csprng_creation<128>)
  ->Name("deterministic_csprng/128b/create")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_deterministic_csprng_creation<192>)
  ->Name("deterministic_csprng/192b/create")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_deterministic_csprng_creation<256>)
  ->Name("deterministic_csprng/256b/create")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
