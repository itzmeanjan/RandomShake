#include "randomshake/randomshake.hpp"
#include "test_utils.hpp"
#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <vector>

static constexpr size_t GENERATED_RANDOM_BYTE_LEN = 1'024 * 1'024; // = 1MB

template<size_t bit_security_level>
static void
expect_same_output_for_deterministic_csprng_using_same_seed()
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  auto rand_gen = [&]() {
    randomshake::randomshake_t<bit_security_level> csprng(seed);
    return csprng();
  };

  std::vector<uint8_t> rand_bytes_a(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> rand_bytes_b(GENERATED_RANDOM_BYTE_LEN, 0xff);

  std::ranges::generate(rand_bytes_a, rand_gen);
  std::ranges::generate(rand_bytes_b, rand_gen);

  EXPECT_EQ(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_Produces_Eq_Output_For_128b_Security)
{
  expect_same_output_for_deterministic_csprng_using_same_seed<128>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_Produces_Eq_Output_For_192b_Security)
{
  expect_same_output_for_deterministic_csprng_using_same_seed<192>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_Produces_Eq_Output_For_256b_Security)
{
  expect_same_output_for_deterministic_csprng_using_same_seed<256>();
}

template<size_t bit_security_level>
static void
expect_diff_output_for_deterministic_csprng_using_diff_seed()
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  auto rand_gen = [&]() {
    randomshake::randomshake_t<bit_security_level> csprng(seed);
    return csprng();
  };

  std::vector<uint8_t> rand_bytes_a(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> rand_bytes_b(GENERATED_RANDOM_BYTE_LEN, 0x00);

  std::ranges::generate(rand_bytes_a, rand_gen);

  randomshake_test_utils::do_bitflip(seed[0], 3);
  std::ranges::generate(rand_bytes_b, rand_gen);

  EXPECT_NE(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_Produces_Ne_Output_For_128b_Security)
{
  expect_diff_output_for_deterministic_csprng_using_diff_seed<128>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_Produces_Ne_Output_For_192b_Security)
{
  expect_diff_output_for_deterministic_csprng_using_diff_seed<192>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_Produces_Ne_Output_For_256b_Security)
{
  expect_diff_output_for_deterministic_csprng_using_diff_seed<256>();
}
