#include "randomshake/randomshake.hpp"
#include <cstdlib>
#include <vector>

int
main()
{
  randomshake::randomshake_t csprng;

  constexpr size_t RANDOM_OUTPUT_BYTE_LEN = 1'024 * 1'024;
  std::vector<uint8_t> rand_byte_seq(RANDOM_OUTPUT_BYTE_LEN, 0);

  csprng.generate(rand_byte_seq);

  return EXIT_SUCCESS;
}
