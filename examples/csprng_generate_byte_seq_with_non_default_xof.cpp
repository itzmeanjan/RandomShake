#include "randomshake/randomshake.hpp"
#include <cstdlib>
#include <vector>

int
main()
{
  /**
   * By default, RandomSHAKE CSPRNG relies on TurboSHAKE256 XOF. But we can specify SHAKE256 to override
   * that default choice. Though remember, the default XOF choice is faster. It can have almost double
   * throughput compared to SHAKE256.
   */
  randomshake::randomshake_t<uint8_t, randomshake::xof_kind_t::SHAKE256> csprng;

  constexpr size_t RANDOM_OUTPUT_BYTE_LEN = 1'024 * 1'024;
  std::vector<uint8_t> rand_byte_seq(RANDOM_OUTPUT_BYTE_LEN, 0);

  csprng.generate(rand_byte_seq);

  return EXIT_SUCCESS;
}
