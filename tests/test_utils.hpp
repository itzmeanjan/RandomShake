#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace randomshake_test_utils {

/**
    Given a byte, this routine flips bit at specified index i, mutating input.
    If i is not a valid index, it is a no-op.
 */
static constexpr void
do_bitflip(uint8_t& val, const size_t bit_idx)
{
  constexpr auto T_bw = std::numeric_limits<std::remove_reference_t<decltype(val)>>::digits;
  if (bit_idx >= T_bw) {
    return;
  }

  const auto hi_bit_mask = 0xffU << (bit_idx + 1);
  const auto lo_bit_mask = 0xffU >> (T_bw - bit_idx);

  const auto selected_bit = (val >> bit_idx) & 0b1U;
  const auto selected_bit_flipped = ~selected_bit & 0b1U;

  val = static_cast<uint8_t>((val & hi_bit_mask) ^ (selected_bit_flipped << bit_idx) ^ (val & lo_bit_mask));
}

}
