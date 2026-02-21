#include "randomshake/randomshake.hpp"
#include "test_consts.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

// A dummy CSPRNG based on specified XOF choice, without ratcheting. Only squeezing after finalizing the sponge.
template<randomshake::xof_kind_t xof_kind = randomshake::xof_kind_t::TURBOSHAKE256>
struct dummy_noratchet_csprng
{
private:
  randomshake::xof_selector_t<xof_kind>::type state;

public:
  explicit dummy_noratchet_csprng(std::span<const uint8_t, randomshake::xof_selector_t<xof_kind>::seed_byte_len> seed)
  {
    state.reset();
    state.absorb(seed);
    state.finalize();
  }

  uint8_t operator()()
  {
    uint8_t result = 0;
    auto res_span = std::span<uint8_t>(&result, 1);

    state.squeeze(res_span);
    return result;
  }
};

namespace {

template<randomshake::xof_kind_t xof_kind>
void
test_ratchet_getting_activated_post_ratchet_period_bytes_output()
{
  // --- Paint output buffers. ---

  /**
   * In following section, we paint subspans of two byte arrays with different byte patterns.
   *
   * We have two equal sized byte arrays, holding pseudo-random output from two different CSPRNGS.
   * One is a RandomSHAKE CSPRNG while another one is a dummy CSPRNG based on same XOF, but no ratcheting.
   */
  std::vector<uint8_t> original_csprng_bytes(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> dummy_noratchet_csprng_bytes(GENERATED_RANDOM_BYTE_LEN, 0x00);

  auto original_csprng_bytes_span = std::span(original_csprng_bytes);
  auto dummy_noratchet_csprng_bytes_span = std::span(dummy_noratchet_csprng_bytes);

  /**
   * After producing these many bytes, the RandomSHAKE CSPRNG should ratchet. But the dummy one should not.
   *
   * 1) Hence till these many bytes, both CSPRNG's should produce exact same byte stream. So initially we paint
   * this portion of two buffers with different byte patterns to be able to catch both CSPRNG's producing same output stream.
   *
   * 2) And after these many bytes, their output should completely diverge. To detect ratcheting kick-in,
   * we paint this portion of two buffers with same byte pattern.
   */
  constexpr auto RATCHET_PERIOD_BYTE_LEN = randomshake::xof_selector_t<xof_kind>::ratchet_period_byte_len;

  auto first_of_original = original_csprng_bytes_span.template first<RATCHET_PERIOD_BYTE_LEN>();
  std::fill(first_of_original.begin(), first_of_original.end(), 0x11);

  auto first_of_dummy = dummy_noratchet_csprng_bytes_span.template first<RATCHET_PERIOD_BYTE_LEN>();
  std::fill(first_of_dummy.begin(), first_of_dummy.end(), 0x22);

  auto last_of_original = original_csprng_bytes_span.template last<GENERATED_RANDOM_BYTE_LEN - RATCHET_PERIOD_BYTE_LEN>();
  std::fill(last_of_original.begin(), last_of_original.end(), 0xff);

  auto last_of_dummy = dummy_noratchet_csprng_bytes_span.template last<GENERATED_RANDOM_BYTE_LEN - RATCHET_PERIOD_BYTE_LEN>();
  std::fill(last_of_dummy.begin(), last_of_dummy.end(), 0xff);

  EXPECT_FALSE(std::ranges::equal(first_of_original, first_of_dummy));
  EXPECT_TRUE(std::ranges::equal(last_of_original, last_of_dummy));

  // --- Painting done and tested to be working. ---

  std::array<uint8_t, randomshake::randomshake_t<uint8_t, xof_kind>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<uint8_t, xof_kind> original_csprng(seed);
  dummy_noratchet_csprng<xof_kind> dummy_noratchet_csprng(seed);

  std::ranges::generate(original_csprng_bytes_span, [&]() { return original_csprng(); });
  std::ranges::generate(dummy_noratchet_csprng_bytes_span, [&]() { return dummy_noratchet_csprng(); });

  // During final testing to detect if ratcheting kicked in, we flip the assertions.

  EXPECT_TRUE(std::ranges::equal(first_of_original, first_of_dummy));
  EXPECT_FALSE(std::ranges::equal(last_of_original, last_of_dummy));
}

}

TEST(RandomSHAKE, Deterministic_CSPRNG_Detect_Ratchet_Working_For_SHAKE256_XOF)
{
  test_ratchet_getting_activated_post_ratchet_period_bytes_output<randomshake::xof_kind_t::SHAKE256>();
}

TEST(RandomSHAKE, Deterministic_CSPRNG_Detect_Ratchet_Working_For_TurboSHAKE256_XOF)
{
  test_ratchet_getting_activated_post_ratchet_period_bytes_output<randomshake::xof_kind_t::TURBOSHAKE256>();
}
