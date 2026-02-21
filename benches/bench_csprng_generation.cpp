#include "bench_utils.hpp"
#include "randomshake/randomshake.hpp"
#include <array>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <vector>

namespace {

template<typename result_type>
void
bench_csprng_output_generation(benchmark::State& state)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<result_type> csprng(seed);
  result_type result{};

  for (auto _itr : state) {
    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(result);

    result ^= csprng();

    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
  }

  state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(sizeof(result_type)));
}

template<randomshake::xof_kind_t xof_kind>
void
bench_csprng_byte_sequence_squeezing(benchmark::State& state)
{
  std::array<uint8_t, randomshake::randomshake_t<uint8_t, xof_kind>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<uint8_t, xof_kind> csprng(seed);

  constexpr size_t RANDOM_OUTPUT_BYTE_LEN = 1'024UL * 1'024UL; // 1 MB
  std::vector<uint8_t> rand_byte_seq(RANDOM_OUTPUT_BYTE_LEN, 0);

  for (auto _itr : state) {
    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(rand_byte_seq);

    csprng.generate(rand_byte_seq);

    benchmark::DoNotOptimize(&csprng);
    benchmark::DoNotOptimize(rand_byte_seq);
    benchmark::ClobberMemory();
  }

  state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(rand_byte_seq.size()));
}

}

BENCHMARK(bench_csprng_output_generation<uint8_t>)->Name("csprng/generate_u8")->ComputeStatistics("min", compute_min)->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_output_generation<uint16_t>)->Name("csprng/generate_u16")->ComputeStatistics("min", compute_min)->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_output_generation<uint32_t>)->Name("csprng/generate_u32")->ComputeStatistics("min", compute_min)->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_output_generation<uint64_t>)->Name("csprng/generate_u64")->ComputeStatistics("min", compute_min)->ComputeStatistics("max", compute_max);

BENCHMARK(bench_csprng_byte_sequence_squeezing<randomshake::xof_kind_t::SHAKE256>)
  ->Name("csprng/shake256/generate_byte_seq")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(bench_csprng_byte_sequence_squeezing<randomshake::xof_kind_t::TURBOSHAKE256>)
  ->Name("csprng/turboshake256/generate_byte_seq")
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
