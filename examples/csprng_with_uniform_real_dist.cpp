#include "randomshake/randomshake.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <random>
#include <span>

namespace {

float
compute_mean(std::span<float> vals)
{
  float res = 0.F;
  for (auto val : vals) {
    res += val;
  }

  return res / static_cast<float>(vals.size());
}

float
compute_standard_deviation(std::span<float> vals)
{
  const auto mean = compute_mean(vals);

  float squared_diff = 0.F;
  for (auto val : vals) {
    squared_diff += std::pow(val - mean, 2.F);
  }

  const auto squared_diff_mean = squared_diff / static_cast<float>(vals.size());
  const auto standard_deviation = std::sqrt(squared_diff_mean);

  return standard_deviation;
}

float
expected_standard_deviation_for_continuous_uniform_distributed_real_numbers(const float start_interval, const float end_interval)
{
  return (end_interval - start_interval) / std::sqrt(12.F);
}

}

int
main()
{
  const float start_interval = 0.F;
  const float end_interval = 1.F;

  assert(start_interval < end_interval);

  randomshake::randomshake_t csprng;
  std::uniform_real_distribution<float> dist{ start_interval, end_interval };

  constexpr size_t NUMBER_OF_RANDOM_FLOATS = 1'000'000;
  std::vector<float> rand_floats(NUMBER_OF_RANDOM_FLOATS, 0.F);
  std::ranges::generate(rand_floats, [&]() { return dist(csprng); });

  const auto computed_sd = compute_standard_deviation(rand_floats);
  const auto expected_sd = expected_standard_deviation_for_continuous_uniform_distributed_real_numbers(start_interval, end_interval);
  const auto difference = std::abs(computed_sd - expected_sd);

  std::cout << "Computed Standard Deviation: " << computed_sd << "\n";
  std::cout << "Expected Standard Deviation: " << expected_sd << "\n";
  std::cout << "Absolute Difference        : " << difference << "\n";

  return EXIT_SUCCESS;
}
