#include "randomshake/randomshake.hpp"
#include "test_consts.hpp"
#include "test_utils.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_Produces_Eq_Output)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  std::vector<uint8_t> rand_bytes_a(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> rand_bytes_b(GENERATED_RANDOM_BYTE_LEN, 0xff);

  randomshake::randomshake_t<> csprng_a(seed);
  std::ranges::generate(rand_bytes_a, [&]() { return csprng_a(); });

  randomshake::randomshake_t<> csprng_b(seed);
  std::ranges::generate(rand_bytes_b, [&]() { return csprng_b(); });

  EXPECT_EQ(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Diff_Seed_Produces_Ne_Output)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  std::vector<uint8_t> rand_bytes_a(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> rand_bytes_b(GENERATED_RANDOM_BYTE_LEN, 0x00);

  randomshake::randomshake_t<> csprng_a(seed);
  std::ranges::generate(rand_bytes_a, [&]() { return csprng_a(); });

  seed.fill(0xde);
  randomshake_test_utils::do_bitflip(seed[0], 3);

  randomshake::randomshake_t<> csprng_b(seed);
  std::ranges::generate(rand_bytes_b, [&]() { return csprng_b(); });

  EXPECT_NE(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_XOF_Kind_Produces_Ne_Output)
{
  std::array<uint8_t, randomshake::randomshake_t<uint8_t, randomshake::xof_kind_t::SHAKE256>::seed_byte_len> seed_a{};
  seed_a.fill(0xde);

  std::vector<uint8_t> rand_bytes_a(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> rand_bytes_b(GENERATED_RANDOM_BYTE_LEN, 0x00);

  randomshake::randomshake_t<uint8_t, randomshake::xof_kind_t::SHAKE256> csprng_a(seed_a);
  std::ranges::generate(rand_bytes_a, [&]() { return csprng_a(); });

  std::array<uint8_t, randomshake::randomshake_t<uint8_t, randomshake::xof_kind_t::TURBOSHAKE256>::seed_byte_len> seed_b{};
  seed_b.fill(0xde);

  randomshake::randomshake_t<> csprng_b(seed_b);
  std::ranges::generate(rand_bytes_b, [&]() { return csprng_b(); });

  EXPECT_NE(rand_bytes_a, rand_bytes_b);
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Result_Type_Produces_Eq_Output)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<uint8_t> csprng_u8(seed);
  randomshake::randomshake_t<uint16_t> csprng_u16(seed);
  randomshake::randomshake_t<uint32_t> csprng_u32(seed);
  randomshake::randomshake_t<uint64_t> csprng_u64(seed);

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
  std::span<const uint8_t> generated_rand_u16_span(reinterpret_cast<uint8_t*>(generated_rand_u16.data()), GENERATED_RANDOM_BYTE_LEN);
  std::span<const uint8_t> generated_rand_u32_span(reinterpret_cast<uint8_t*>(generated_rand_u32.data()), GENERATED_RANDOM_BYTE_LEN);
  std::span<const uint8_t> generated_rand_u64_span(reinterpret_cast<uint8_t*>(generated_rand_u64.data()), GENERATED_RANDOM_BYTE_LEN);

  EXPECT_TRUE(std::ranges::equal(generated_rand_u8_span, generated_rand_u16_span));
  EXPECT_TRUE(std::ranges::equal(generated_rand_u16_span, generated_rand_u32_span));
  EXPECT_TRUE(std::ranges::equal(generated_rand_u32_span, generated_rand_u64_span));
}

TEST(RandomShake, Deterministic_CSPRNG_Using_Same_Seed_With_Diff_Public_API)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<> csprng_u8{ seed };
  randomshake::randomshake_t<> csprng_bytes{ seed };

  std::vector<uint8_t> generated_rand_u8(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> generated_byte_seq(GENERATED_RANDOM_BYTE_LEN, 0xff);

  std::ranges::generate(generated_rand_u8, [&]() { return csprng_u8(); }); // Squeezes one byte at a time
  csprng_bytes.generate(generated_byte_seq);                               // Squeezes arbitrary many bytes at a time

  EXPECT_EQ(generated_rand_u8, generated_byte_seq);
}

TEST(RandomShake, Deterministic_CSPRNG_Oneshot_vs_Multishot_Squeezing)
{
  std::array<uint8_t, randomshake::randomshake_t<>::seed_byte_len> seed{};
  seed.fill(0xde);

  randomshake::randomshake_t<> csprng_oneshot{ seed };
  randomshake::randomshake_t<> csprng_multishot{ seed };

  std::vector<uint8_t> generated_bytes_oneshot(GENERATED_RANDOM_BYTE_LEN, 0x00);
  std::vector<uint8_t> generated_bytes_multishot(GENERATED_RANDOM_BYTE_LEN, 0xff);

  auto generated_bytes_oneshot_span = std::span(generated_bytes_oneshot);
  auto generated_bytes_multishot_span = std::span(generated_bytes_multishot);

  // Squeeze all random bytes in a single go
  csprng_oneshot.generate(generated_bytes_oneshot_span);

  // Squeeze random bytes in multiple calls
  {
    const size_t out_byte_len = generated_bytes_multishot.size();
    size_t out_offset = 0;

    while (out_offset < out_byte_len) {
      csprng_multishot.generate(generated_bytes_multishot_span.subspan(out_offset, 1));
      out_offset++;

      const auto next_squeeze_byte_len = std::min<size_t>(generated_bytes_multishot_span[out_offset], // Value held by last byte squeezed
                                                          out_byte_len - out_offset);                 // How many bytes are yet to be squeezed

      csprng_multishot.generate(generated_bytes_multishot_span.subspan(out_offset, next_squeeze_byte_len));
      out_offset += next_squeeze_byte_len;
    }
  }

  EXPECT_EQ(generated_bytes_oneshot, generated_bytes_multishot);
}
