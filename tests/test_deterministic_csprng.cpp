#include "randomshake/randomshake.hpp"
#include "test_utils.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
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

template<size_t bit_security_level>
static void
expect_same_output_for_deterministic_csprng_using_same_seed_but_diff_result_type()
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<bit_security_level, uint8_t> csprng_u8(seed);
  randomshake::randomshake_t<bit_security_level, uint16_t> csprng_u16(seed);
  randomshake::randomshake_t<bit_security_level, uint32_t> csprng_u32(seed);
  randomshake::randomshake_t<bit_security_level, uint64_t> csprng_u64(seed);

  constexpr size_t num_rand_u8_to_gen = GENERATED_RANDOM_BYTE_LEN;
  constexpr size_t num_rand_u16_to_gen = num_rand_u8_to_gen / 2;
  constexpr size_t num_rand_u32_to_gen = num_rand_u16_to_gen / 2;
  constexpr size_t num_rand_u64_to_gen = num_rand_u32_to_gen / 2;

  std::vector<uint8_t> generated_rand_u8(num_rand_u8_to_gen, 0x00);
  std::vector<uint16_t> generated_rand_u16(num_rand_u16_to_gen, 0x11);
  std::vector<uint32_t> generated_rand_u32(num_rand_u32_to_gen, 0x22);
  std::vector<uint64_t> generated_rand_u64(num_rand_u64_to_gen, 0x33);

  std::ranges::generate(generated_rand_u8, [&]() { return csprng_u8(); });
  std::ranges::generate(generated_rand_u16, [&]() { return csprng_u16(); });
  std::ranges::generate(generated_rand_u32, [&]() { return csprng_u32(); });
  std::ranges::generate(generated_rand_u64, [&]() { return csprng_u64(); });

  std::span<const uint8_t> generated_rand_u8_span(generated_rand_u8);
  std::span<const uint8_t> generated_rand_u16_span(reinterpret_cast<uint8_t*>(generated_rand_u16.data()),
                                                   GENERATED_RANDOM_BYTE_LEN);
  std::span<const uint8_t> generated_rand_u32_span(reinterpret_cast<uint8_t*>(generated_rand_u32.data()),
                                                   GENERATED_RANDOM_BYTE_LEN);
  std::span<const uint8_t> generated_rand_u64_span(reinterpret_cast<uint8_t*>(generated_rand_u64.data()),
                                                   GENERATED_RANDOM_BYTE_LEN);

  EXPECT_TRUE(std::ranges::equal(generated_rand_u8_span, generated_rand_u16_span));
  EXPECT_TRUE(std::ranges::equal(generated_rand_u16_span, generated_rand_u32_span));
  EXPECT_TRUE(std::ranges::equal(generated_rand_u32_span, generated_rand_u64_span));
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Result_Type_Produces_Same_Output_For_128b_Security)
{
  expect_same_output_for_deterministic_csprng_using_same_seed_but_diff_result_type<128>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Result_Type_Produces_Same_Output_For_192b_Security)
{
  expect_same_output_for_deterministic_csprng_using_same_seed_but_diff_result_type<192>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Result_Type_Produces_Same_Output_For_256b_Security)
{
  expect_same_output_for_deterministic_csprng_using_same_seed_but_diff_result_type<256>();
}

template<size_t bit_security_level>
static void
expect_same_output_for_deterministic_csprng_using_different_public_api()
{
  std::array<uint8_t, randomshake::randomshake_t<bit_security_level>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<bit_security_level> csprng_u8{ seed };
  randomshake::randomshake_t<bit_security_level> csprng_bytes{ seed };

  std::vector<uint8_t> generated_rand_u8(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> generated_byte_seq(GENERATED_RANDOM_BYTE_LEN, 0xff);

  std::ranges::generate(generated_rand_u8, [&]() { return csprng_u8(); }); // Squeezes one byte at a time
  csprng_bytes.generate(generated_byte_seq);                               // Squeezes arbitrary many bytes at a time

  EXPECT_EQ(generated_rand_u8, generated_byte_seq);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Public_API_For_128b_Security)
{
  expect_same_output_for_deterministic_csprng_using_different_public_api<128>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Public_API_For_192b_Security)
{
  expect_same_output_for_deterministic_csprng_using_different_public_api<192>();
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Public_API_For_256b_Security)
{
  expect_same_output_for_deterministic_csprng_using_different_public_api<256>();
}
