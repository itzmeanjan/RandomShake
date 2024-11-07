#include "bench_utils.hpp"
#include "randomshake/randomshake.hpp"
#include <array>
#include <benchmark/benchmark.h>

template<size_t bit_security_level, typename result_type>
static void
bench_csprng_output_generation(benchmark::State& state)
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<bit_security_level, result_type> csprng(seed);
  result_type result{};

  for (auto _ : state) {
    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(result);

    result ^= csprng();

    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
  }

  state.SetBytesProcessed(state.iterations() * sizeof(result_type));
}

BENCHMARK(bench_csprng_output_generation<256, uint8_t>)
  ->Name("csprng/256b/generate_u8")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_output_generation<256, uint16_t>)
  ->Name("csprng/256b/generate_u16")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_output_generation<256, uint32_t>)
  ->Name("csprng/256b/generate_u32")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_output_generation<256, uint64_t>)
  ->Name("csprng/256b/generate_u64")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
