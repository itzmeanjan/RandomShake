#include "randomshake/randomshake.hpp"
#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <vector>

static constexpr size_t GENERATED_RANDOM_BYTE_LEN = 1'024 * 1'024; // = 1MB

template<size_t bit_security_level>
static void
test_deterministic_csprng_using_same_seed()
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  std::vector<uint8_t> rand_bytes_a(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> rand_bytes_b(GENERATED_RANDOM_BYTE_LEN, 0xff);

  auto rand_gen = [&]() {
    randomshake::randomshake_t<bit_security_level> csprng(seed);
    return csprng();
  };

  std::ranges::generate(rand_bytes_a, rand_gen);
  std::ranges::generate(rand_bytes_b, rand_gen);

  EXPECT_EQ(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_For_128b_Security)
{
  test_deterministic_csprng_using_same_seed<128>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_For_192b_Security)
{
  test_deterministic_csprng_using_same_seed<192>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_For_256b_Security)
{
  test_deterministic_csprng_using_same_seed<256>();
}

template<size_t bit_security_level>
static void
test_deterministic_csprng_using_diff_seed()
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

  seed[0] = ~seed[0];
  std::ranges::generate(rand_bytes_b, rand_gen);

  EXPECT_NE(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_For_128b_Security)
{
  test_deterministic_csprng_using_diff_seed<128>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_For_192b_Security)
{
  test_deterministic_csprng_using_diff_seed<192>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_For_256b_Security)
{
  test_deterministic_csprng_using_diff_seed<256>();
}
