#include "randomshake/randomshake.hpp"
#include <cstdlib>
#include <random>
#include <ranges>

// This example collects inspiration from https://seth.rocks/articles/cpprandom. See the last code-snippet.
int
main()
{
  randomshake::randomshake_t<> csprng;
  std::binomial_distribution dist{ 1'000, .5 };

  for (auto _ : std::ranges::iota_view{ 1, 10 }) {
    std::cout << "[BINOMIAL dISTRIBUTION] Number of heads in 1,000 flips: " << dist(csprng) << '\n';
  }

  return EXIT_SUCCESS;
}
