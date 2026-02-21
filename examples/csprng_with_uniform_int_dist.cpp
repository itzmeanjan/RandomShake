#include "randomshake/randomshake.hpp"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <random>

int
main()
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xfe);

  // Deterministic CSPRNG : Seed -based initialization of CSPRNG
  randomshake::randomshake_t csprng(seed);
  std::uniform_int_distribution<uint8_t> dist{ 97, 102 };

  for (int idx = 1; idx < 10; ++idx) {
    std::cout << "[UNIFORM iNT dISTRIBUTION] Random integer sampled in [97, 102]: " << dist(csprng) << '\n';
  }

  return EXIT_SUCCESS;
}
