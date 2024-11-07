#include "randomshake/randomshake.hpp"
#include <cstdint>
#include <cstdlib>
#include <random>
#include <ranges>

int
main()
{
  randomshake::randomshake_t<256> csprng;
  std::uniform_int_distribution<uint8_t> dist{ 97, 102 };

  for (auto _ : std::ranges::iota_view{ 1, 10 }) {
    std::cout << "[UNIFORM iNT dISTRIBUTION] Random integer sampled in [97, 102]: " << dist(csprng) << '\n';
  }

  return EXIT_SUCCESS;
}
