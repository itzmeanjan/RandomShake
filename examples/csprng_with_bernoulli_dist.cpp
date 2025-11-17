#include "randomshake/randomshake.hpp"
#include <cstdlib>
#include <iomanip>
#include <map>
#include <random>
#include <ranges>

// This example collects inspiration from https://en.cppreference.com/w/cpp/numeric/random/bernoulli_distribution.
// See the example section.
int
main()
{
  randomshake::randomshake_t csprng;
  std::bernoulli_distribution dist{ .25 };

  std::map<bool, size_t> hist;
  for (auto _ : std::ranges::iota_view{ 0, 10'000 }) {
    ++hist[dist(csprng)];
  }

  std::cout << "\nBERNOULLI dISTRIBUTION :\n";
  std::cout << std::boolalpha;
  for (auto const& [key, value] : hist) {
    std::cout << std::setw(5) << key << ' ' << std::string(value / 500, '*') << '\n';
  }

  return EXIT_SUCCESS;
}
