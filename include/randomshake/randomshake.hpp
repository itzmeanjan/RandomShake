#pragma once
#include "sha3/internals/force_inline.hpp"
#include "sha3/shake256.hpp"
#include "sha3/turboshake256.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>

namespace randomshake {

// Compile-time check to ensure that endianness is little, as correctness of program depends on it.
consteval auto
check_endianness()
{
  return std::endian::native == std::endian::little;
}

/**
 * Ensures that value is materialized (and not optimized away), but doesn't clobber memory, like google-benchmark does.
 * Taken from https://theunixzoo.co.uk/blog/2021-10-14-preventing-optimisations.html.
 */
template<typename Tp>
forceinline void
DoNotOptimize(Tp& value)
{
  asm volatile("" : "+r,m"(value) : :);
}

// Enum listing supported eXtendable Output Functions (XOFs), which can be used for producing pseudo-random byte stream.
enum class xof_kind_t : uint8_t
{
  SHAKE256,      // Based on 24-rounds keccak permutation.
  TURBOSHAKE256, // Based on 12-rounds keccak permutation. Almost doubles the throughput. The default choice.
};

// A helper trait, for selecting which XOF to use in RandomSHAKE CSPRNG.
template<xof_kind_t xof_kind>
struct xof_selector_t
{};

// Specialization for SHAKE256 XOF.
template<>
struct xof_selector_t<xof_kind_t::SHAKE256>
{
  using type = shake256::shake256_t;

  // Bit width of the rate portion of keccak sponge for SHAKE256 XOF.
  static constexpr size_t rate = shake256::RATE;

  // Required seed byte length to initialize the SHAKE256 XOF.
  static constexpr size_t seed_byte_len = rate / std::numeric_limits<uint8_t>::digits;

  /**
   * Everytime these many bytes are squeezed from the underlying keccak sponge,
   * we zeroize first `ratchet_byte_len`-bytes of Keccak permutation state and re-apply 24-rounds permutation.
   *
   * Ratchet period gets computed as `8 x RATE-of-the-underlying-keccak-sponge` bits.
   */
  static constexpr size_t ratchet_period_byte_len = shake256::RATE;

  // First these many bytes of keccak permutation are zeroized during ratcheting.
  static constexpr size_t ratchet_byte_len = shake256::TARGET_BIT_SECURITY_LEVEL / std::numeric_limits<uint8_t>::digits;
};

// Specialization for TurboSHAKE256 XOF.
template<>
struct xof_selector_t<xof_kind_t::TURBOSHAKE256>
{
  using type = turboshake256::turboshake256_t;

  // Bit width of the rate portion of keccak sponge for TurboSHAKE256 XOF.
  static constexpr size_t rate = turboshake256::RATE;

  // Required seed byte length to initialize the TurboSHAKE256 XOF.
  static constexpr size_t seed_byte_len = rate / std::numeric_limits<uint8_t>::digits;

  /**
   * Everytime these many bytes are squeezed from the underlying keccak sponge,
   * we zeroize first `ratchet_byte_len`-bytes of Keccak permutation state and re-apply 12-rounds permutation.
   *
   * Ratchet period gets computed as `8 x RATE-of-the-underlying-keccak-sponge` bits.
   */
  static constexpr size_t ratchet_period_byte_len = turboshake256::RATE;

  // First these many bytes of keccak permutation state are zeroized during ratcheting.
  static constexpr size_t ratchet_byte_len = turboshake256::TARGET_BIT_SECURITY_LEVEL / std::numeric_limits<uint8_t>::digits;
};

/**
 * RandomSHAKE - TurboSHAKE256 (by default) or SHAKE256-backed Cryptographically Secure Pseudo-Random Number Generator (CSPRNG).
 *
 * Allowing both (a) `std::random_device` sampled seed, (b) User provided seed-based initialization of CSPRNG.
 * After every `ratchet_period_byte_len`-many bytes are squeezed from the underlying XOF instance, we perform
 * ratcheting i.e. zeroing out of first `ratchet_byte_len` -many bytes of Keccak permutation state and re-applying
 * permutation.
 *
 * Design of RandomSHAKE CSPRNG API collects inspiration from https://seth.rocks/articles/cpprandom.
 */
template<typename UIntType = uint8_t, xof_kind_t xof_kind = xof_kind_t::TURBOSHAKE256>
  requires(std::is_unsigned_v<UIntType> && check_endianness())
struct randomshake_t
{
private:
  xof_selector_t<xof_kind>::type state{};
  std::array<uint8_t, xof_selector_t<xof_kind>::ratchet_period_byte_len> buffer{};
  size_t buffer_offset = 0u;

public:
  using result_type = UIntType;

  static constexpr auto seed_byte_len = xof_selector_t<xof_kind>::seed_byte_len;
  static constexpr auto min = std::numeric_limits<result_type>::min;
  static constexpr auto max = std::numeric_limits<result_type>::max;

  /**
   * Samples `seed_byte_len` -many bytes from std::random_device and initializes the chosen XOF - making it ready for use.
   * Before you use this constructor, I strongly advise you to read https://en.cppreference.com/w/cpp/numeric/random/random_device.
   */
  forceinline randomshake_t()
  {
    std::array<uint8_t, seed_byte_len> seed{};
    auto seed_span = std::span(seed);

    std::random_device rd{};
    const auto entropy = rd.entropy();
    if (entropy == 0.) {
      std::cout << "[RANDOMSHAKE WARNING] Non-deterministic seed generator has zero entropy ! "
                   "Read https://en.cppreference.com/w/cpp/numeric/random/random_device/entropy for more insight.\n";
    }

    constexpr size_t step_by = sizeof(std::random_device::result_type);
    size_t seed_offset = 0;
    while (seed_offset < seed_span.size()) {
      const auto v = rd();

      static_assert(seed_byte_len % step_by == 0, "Seed byte length must be a multiple of `step_by`, for following memcpy to work correctly !");
      std::memcpy(seed_span.subspan(seed_offset, step_by).data(), reinterpret_cast<const uint8_t*>(&v), step_by);

      seed_offset += step_by;
    }

    state.reset();
    state.absorb(seed_span);
    state.finalize();
    state.squeeze(buffer);
  }

  /**
   * Explicit constructor. Expects user to supply us with `seed_byte_len` -bytes seed, which is used for initializing
   * the chosen XOF. It is user's responsibility to ensure that the supplied seed has sufficient entropy.
   */
  forceinline explicit constexpr randomshake_t(std::span<const uint8_t, seed_byte_len> seed)
  {
    state.reset();
    state.absorb(seed);
    state.finalize();
    state.squeeze(buffer);
  }

  // Delete copy and move constructors - as this CSPRNG instance is neither copyable nor movable.
  randomshake_t(const randomshake_t&) = delete;
  randomshake_t(randomshake_t&&) = delete;
  randomshake_t& operator=(const randomshake_t&) = delete;
  randomshake_t& operator=(randomshake_t&&) = delete;

  // Zeroize internal state when destroying an instance of CSPRNG.
  ~randomshake_t()
  {
    state.reset();
    DoNotOptimize(state);

    buffer.fill(0);
    DoNotOptimize(buffer);

    buffer_offset = 0;
  }

  // Squeezes a random value of type `result_type`.
  [[nodiscard("Internal state of CSPRNG has changed, you should consume this value")]] forceinline result_type operator()()
  {
    constexpr size_t required_num_bytes = sizeof(result_type);
    const size_t readble_num_bytes = buffer.size() - buffer_offset;

    static_assert(xof_selector_t<xof_kind>::ratchet_period_byte_len % required_num_bytes == 0,
                  "Buffer size nust be a multiple of `required_num_bytes`, for following ratchet()->squeeze() to work correctly !");

    // When the buffer is exhausted, it's time to ratchet and fill the buffer with new ready-to-use random bytes.
    if (readble_num_bytes == 0) {
      state.ratchet(xof_selector_t<xof_kind>::ratchet_byte_len);
      state.squeeze(buffer);
      buffer_offset = 0;
    }

    result_type result{};

    auto src_ptr = reinterpret_cast<const uint8_t*>(buffer.data()) + buffer_offset;
    auto dst_ptr = reinterpret_cast<uint8_t*>(&result);

    std::memcpy(dst_ptr, src_ptr, required_num_bytes);
    buffer_offset += required_num_bytes;

    return result;
  }

  // Squeezes n(>=0) random bytes, instead of getting one at a time, as done by the above functor.
  forceinline void generate(std::span<uint8_t> output)
  {
    size_t out_offset = 0;

    while (out_offset < output.size()) {
      const size_t readable_num_bytes = buffer.size() - buffer_offset;
      const size_t required_num_bytes = output.size() - out_offset;
      const size_t copyable_num_bytes = std::min(readable_num_bytes, required_num_bytes);

      auto src_ptr = reinterpret_cast<const uint8_t*>(buffer.data()) + buffer_offset;
      auto dst_ptr = reinterpret_cast<uint8_t*>(output.data()) + out_offset;

      std::memcpy(dst_ptr, src_ptr, copyable_num_bytes);

      buffer_offset += copyable_num_bytes;
      out_offset += copyable_num_bytes;

      if (buffer_offset == buffer.size()) {
        state.ratchet(xof_selector_t<xof_kind>::ratchet_byte_len);
        state.squeeze(buffer);
        buffer_offset = 0;
      }
    }
  }
};

}
