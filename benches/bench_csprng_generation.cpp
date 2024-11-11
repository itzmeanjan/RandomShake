#include "bench_utils.hpp"
#include "randomshake/randomshake.hpp"
#include <array>
#include <benchmark/benchmark.h>
#include <vector>

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

template<size_t bit_security_level>
static void
bench_csprng_byte_sequence_squeezing(benchmark::State& state)
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<bit_security_level> csprng(seed);

  constexpr size_t RANDOM_OUTPUT_BYTE_LEN = 1'024 * 1'024; // 1 MB
  std::vector<uint8_t> rand_byte_seq(RANDOM_OUTPUT_BYTE_LEN, 0);

  for (auto _ : state) {
    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(rand_byte_seq);

    csprng.generate(rand_byte_seq);

    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(rand_byte_seq);
    benchmark::ClobberMemory();
  }

  state.SetBytesProcessed(state.iterations() * rand_byte_seq.size());
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

BENCHMARK(bench_csprng_byte_sequence_squeezing<128>)
  ->Name("csprng/128b/generate_byte_seq")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_byte_sequence_squeezing<192>)
  ->Name("csprng/192b/generate_byte_seq")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_byte_sequence_squeezing<256>)
  ->Name("csprng/256b/generate_byte_seq")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
