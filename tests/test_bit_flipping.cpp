#include "test_utils.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

TEST(RandomSHAKETestUtility, Bit_Flipping_Must_Work_Correctly)
{
  auto input_word = std::numeric_limits<uint8_t>::min();
  constexpr auto T_bw = std::numeric_limits<decltype(input_word)>::digits;

  // 0x00 -> 0xff, by flipping each bit, one at a time
  for (size_t i = 0; i < T_bw; i++) {
    randomshake_test_utils::do_bitflip(input_word, i);
  }

  randomshake_test_utils::do_bitflip(input_word, T_bw);
  EXPECT_EQ(input_word, std::numeric_limits<decltype(input_word)>::max());

  // 0xff -> 0x00, by flipping each bit, one at a time
  for (size_t i = 0; i < T_bw; i++) {
    randomshake_test_utils::do_bitflip(input_word, i);
  }

  randomshake_test_utils::do_bitflip(input_word, T_bw);
  EXPECT_EQ(input_word, std::numeric_limits<decltype(input_word)>::min());
}
