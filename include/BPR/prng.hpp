/**
 * Copyright (c) 2024 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event 
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial 
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you 
 *   wrote the original software. If you use this software in a product, an acknowledgment 
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef BPR_PRNG_HPP
#define BPR_PRNG_HPP

#include "engine.hpp"
#include "utils.hpp"
#include <cstdint>
#include <array>

namespace bpr { namespace prng {

/**
 * @class Xorshift128Plus
 * @brief A fast and simple PRNG based on the xorshift family, specifically the Xorshift128Plus algorithm.
 * 
 * This PRNG is a variation of the xorshift algorithm and is known for being very fast, with relatively good statistical properties.
 * It is suitable for applications requiring quick random numbers with a reasonable degree of randomness.
 * 
 * @note Strengths:
 * - Very fast and efficient.
 * - Simple and small state (2 x 64-bit values).
 * 
 * @note Weaknesses:
 * - Not suitable for cryptographic purposes due to its relatively simple structure and predictability.
 * - May exhibit weak statistical properties compared to more advanced algorithms.
 * 
 * @see Xorshift128Plus (https://en.wikipedia.org/wiki/Xorshift#xorshift+)
 */
class Xorshift128Plus : public IEngine<uint64_t, 2>
{
public:
    explicit constexpr Xorshift128Plus(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1)
        })
    { }

    constexpr uint64_t next() noexcept override {
        uint64_t s1 = m_state[0];
        uint64_t s0 = m_state[1];
        m_state[0] = s0;
        s1 ^= s1 << 23;
        s1 ^= s1 >> 17;
        s1 ^= s0;
        s1 ^= s0 >> 26;
        m_state[1] = s1;
        return m_state[1] + s0;
    }
};

/**
 * @class Xoroshiro128Plus
 * @brief A high-quality PRNG based on the Xoroshiro128Plus algorithm, a faster version of xorshift.
 * 
 * Xoroshiro128Plus offers better randomness properties than Xorshift128Plus, while still maintaining high performance.
 * It is a good choice for simulations, games, and non-cryptographic applications where quality random numbers are required.
 * 
 * @note Strengths:
 * - High quality random numbers with better statistical properties than Xorshift128Plus.
 * - Very fast and efficient.
 * 
 * @note Weaknesses:
 * - Not suitable for cryptography.
 * - Slightly more complex than Xorshift128Plus.
 * 
 * @see Xoroshiro128+ (https://en.wikipedia.org/wiki/Xorshift#xoroshiro)
 */
class Xoroshiro128Plus : public IEngine<uint64_t, 2>
{
public:
    explicit constexpr Xoroshiro128Plus(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1)
        })
    { }

    constexpr uint64_t next() noexcept override {
        uint64_t s0 = m_state[0];
        uint64_t s1 = m_state[1];
        uint64_t result = s0 + s1;
        s1 ^= s0;
        m_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
        m_state[1] = rotl(s1, 36);
        return result;
    }
};

/**
 * @class Xoshiro256Star
 * @brief A fast, high-quality PRNG based on the Xoshiro256Star algorithm.
 * 
 * Xoshiro256Star is part of the Xoshiro family of PRNGs and offers excellent randomness properties with very good performance.
 * It is well-suited for applications that require high-quality random numbers for large-scale simulations or gaming.
 * 
 * @note Strengths:
 * - Very high-quality random numbers with excellent statistical properties.
 * - Highly efficient and fast.
 * - Suitable for large-scale applications and simulations.
 * 
 * @note Weaknesses:
 * - More complex than other PRNGs (larger state and more computations).
 * - Not suitable for cryptographic purposes.
 * 
 * @see Xoshiro256 (https://en.wikipedia.org/wiki/Xorshift#xoshiro)
 */
class Xoshiro256Star : public IEngine<uint64_t, 4>
{
public:
    explicit constexpr Xoshiro256Star(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1),
            splitmix64(seed + 2),
            splitmix64(seed + 3)
        })
    { }

    constexpr uint64_t next() noexcept override {
        const uint64_t result = rotl(m_state[1] * 5, 7) * 9;
        const uint64_t t = m_state[1] << 17;
        m_state[2] ^= m_state[0];
        m_state[3] ^= m_state[1];
        m_state[1] ^= m_state[2];
        m_state[0] ^= m_state[3];
        m_state[2] ^= t;
        m_state[3] = rotl(m_state[3], 45);
        return result;
    }
};

/**
 * @class PCG32
 * @brief A high-quality PRNG based on the Permuted Congruential Generator (PCG) algorithm.
 * 
 * PCG32 is known for its excellent statistical properties and is a good choice for applications requiring robust randomness.
 * It has a compact state (1 x 64-bit value) and provides high-quality random numbers while being fast enough for most applications.
 * 
 * @note Strengths:
 * - Very high-quality random numbers with excellent statistical properties.
 * - Compact state, making it memory-efficient.
 * - Good performance for most non-cryptographic applications.
 * 
 * @note Weaknesses:
 * - Not cryptographically secure.
 * - Slightly more complex than xorshift-based generators.
 * 
 * @see PCG (https://www.pcg-random.org/)
 */
class PCG32 : public IEngine<uint64_t, 1>
{
public:
    explicit constexpr PCG32(uint64_t seed = compile_time()) noexcept
        : IEngine({ 0ULL })
    {
        next32();
        m_state[0] += seed;
        next32();
    }

    constexpr uint64_t next() noexcept {
        uint64_t value = next32();
        value <<= 32;
        value |= next32();
        return value;
    }

    constexpr uint32_t next32() noexcept {
        uint64_t state = m_state[0];
        m_state[0] = state * MUL + INC;

        uint32_t value = (uint32_t)((state ^ (state >> 18)) >> 27);
        int rot = state >> 59;

        return rot ? (value >> rot) | (value << (32 - rot)) : value;
    }

    constexpr void advance(uint64_t delta) noexcept {
        uint64_t cur_mult = MUL;
        uint64_t cur_plus = INC;
        uint64_t acc_mult = 1;
        uint64_t acc_plus = 0;
        while (delta != 0) {
            if (delta & 1) {
                acc_mult *= cur_mult;
                acc_plus = acc_plus * cur_mult + cur_plus;
            }
            cur_plus = (cur_mult + 1) * cur_plus;
            cur_mult *= cur_mult;
            delta >>= 1;
        }
        m_state[0] = acc_mult * m_state[0] + acc_plus;
    }

private:
    static constexpr uint64_t MUL = 6364136223846793005ULL;
    static constexpr uint64_t INC = 1442695040888963407ULL;
};

}} // namespace bpi::prng

#endif // BPR_PRNG_HPP
