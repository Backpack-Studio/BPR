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
 * @class Xoroshiro128p
 * @brief A fast, general-purpose PRNG ideal for games and lightweight simulations.
 * 
 * Best suited for:
 * - Games and real-time applications requiring speed
 * - Simple simulations where basic randomness is sufficient
 * - Scenarios where memory usage must be minimal (128-bit state)
 * 
 * Performance characteristics:
 * - Period: 2^128 - 1
 * - State size: 128 bits
 * - Speed: Extremely fast (2-3 operations per output)
 * 
 * Avoid using for:
 * - Cryptographic purposes
 * - Scientific simulations requiring high precision
 * - Applications sensitive to linear artifacts
 */
class Xoroshiro128p : public IEngine<uint64_t, 2>
{
public:
    explicit constexpr Xoroshiro128p(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1)
        })
    { }

    constexpr uint64_t next() noexcept override {
        const uint64_t s0 = m_state[0];
        uint64_t s1 = m_state[1];
        const uint64_t result = s0 + s1;
        s1 ^= s0;
        m_state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        m_state[1] = rotl(s1, 37);
        return result;
    }
};

/**
 * @class Xoroshiro128pp
 * @brief An enhanced variant of Xoroshiro128+ offering better statistical distribution.
 * 
 * Best suited for:
 * - Numerical simulations requiring good statistical properties
 * - Game systems where quality matters (e.g., procedural generation)
 * - Applications needing a balance of speed and quality
 * 
 * Performance characteristics:
 * - Period: 2^128 - 1
 * - State size: 128 bits
 * - Speed: Very fast (3-4 operations per output)
 * - Superior statistical properties compared to Xoroshiro128p
 * 
 * Trade-offs:
 * - Slightly slower than Xoroshiro128p
 * - Better quality for most applications
 */
class Xoroshiro128pp : public IEngine<uint64_t, 2>
{
public:
    explicit constexpr Xoroshiro128pp(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1)
        })
    { }

    constexpr uint64_t next() noexcept override {
        uint64_t s0 = m_state[0];
        uint64_t s1 = m_state[1];
        uint64_t result = rotl(s0 + s1, 17) + s0;
        s1 ^= s0;
        m_state[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21);
        m_state[1] = rotl(s1, 28);
        return result;
    }
};

/**
 * @class Xoroshiro128ss
 * @brief Scrambled variant of Xoroshiro128 optimized for floating-point conversions.
 * 
 * Best suited for:
 * - Monte Carlo simulations
 * - Applications requiring high-quality floating-point random numbers
 * - Scientific computing where bit patterns matter
 * 
 * Performance characteristics:
 * - Period: 2^128 - 1
 * - State size: 128 bits
 * - Speed: Fast (4-5 operations per output)
 * - Excellent bit mixing properties
 * 
 * Recommended when:
 * - Converting to floating-point is a primary use case
 * - Output bit patterns need to be highly scrambled
 */
class Xoroshiro128ss : public IEngine<uint64_t, 2>
{
public:
    explicit constexpr Xoroshiro128ss(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1)
        })
    { }

    constexpr uint64_t next() noexcept override {
	    const uint64_t s0 = m_state[0];
        uint64_t s1 = m_state[1];
        const uint64_t result = rotl(s0 * 5, 7) * 9;
        s1 ^= s0;
        m_state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        m_state[1] = rotl(s1, 37);
        return result;
    }
};

/**
 * @class Xoshiro256p
 * @brief A high-performance PRNG with larger state space.
 * 
 * Best suited for:
 * - Parallel applications (multiple streams)
 * - Long-running simulations
 * - Cases where period length is critical
 * 
 * Performance characteristics:
 * - Period: 2^256 - 1
 * - State size: 256 bits
 * - Speed: Very fast (similar to Xoroshiro128p)
 * - Good statistical properties for most dimensions
 * 
 * Choose this when:
 * - You need multiple independent streams
 * - Longer period is required than Xoroshiro128 variants
 */
class Xoshiro256p : public IEngine<uint64_t, 4>
{
public:
    explicit constexpr Xoshiro256p(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1),
            splitmix64(seed + 2),
            splitmix64(seed + 3)
        })
    { }

    constexpr uint64_t next() noexcept override {
        const uint64_t result = m_state[0] + m_state[3];
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
 * @class Xoshiro256pp
 * @brief Enhanced version of Xoshiro256p with improved scrambling.
 * 
 * Best suited for:
 * - Complex simulations requiring high statistical quality
 * - Applications where output quality is critical
 * - Scenarios requiring both long period and high dimensional equidistribution
 * 
 * Performance characteristics:
 * - Period: 2^256 - 1
 * - State size: 256 bits
 * - Speed: Fast (slightly slower than Xoshiro256p)
 * - Excellent statistical properties in high dimensions
 * 
 * Recommended when:
 * - Statistical quality is more important than raw speed
 * - Both long period and high quality are required
 */
class Xoshiro256pp : public IEngine<uint64_t, 4>
{
public:
    explicit constexpr Xoshiro256pp(uint64_t seed = compile_time()) noexcept
        : IEngine({
            splitmix64(seed),
            splitmix64(seed + 1),
            splitmix64(seed + 2),
            splitmix64(seed + 3)
        })
    { }

    constexpr uint64_t next() noexcept override {
        const uint64_t result = rotl(m_state[0] + m_state[3], 23) + m_state[0];
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
 * @class Xoshiro256ss
 * @brief Star-Star variant of Xoshiro256 optimized for floating-point conversions and bit mixing.
 * 
 * Best suited for:
 * - Floating-point heavy Monte Carlo simulations
 * - Scientific applications requiring high-quality bit scrambling
 * - Applications sensitive to linear artifacts in lower bits
 * 
 * Performance characteristics:
 * - Period: 2^256 - 1
 * - State size: 256 bits
 * - Speed: Fast (comparable to other Xoshiro256 variants)
 * - Superior bit mixing compared to p/pp variants
 * 
 * Key differences from p/pp variants:
 * - Uses multiplication and rotation for better bit mixing
 * - Significantly higher quality in floating-point conversions
 * - Better suited for applications requiring nonlinear transformations
 * 
 * Choose this when:
 * - Working primarily with floating-point numbers
 * - Need to avoid linear artifacts in output
 * - Statistical quality of bit patterns is critical
 */
class Xoshiro256ss : public IEngine<uint64_t, 4>
{
public:
    explicit constexpr Xoshiro256ss(uint64_t seed = compile_time()) noexcept
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
 * @brief A statistically excellent PRNG with small state space.
 * 
 * Best suited for:
 * - Applications requiring extremely high-quality random numbers
 * - Cases where memory usage must be minimal
 * - Scenarios where predictability must be minimized
 * 
 * Performance characteristics:
 * - Period: 2^64
 * - State size: 64 bits
 * - Speed: Moderately fast (more complex operations than Xoroshiro)
 * - Excellent statistical properties across all dimensions
 * 
 * Unique features:
 * - Supports stream selection for multiple independent sequences
 * - Includes advance/jump operations
 * - Better recovery from poor seeding
 * 
 * Choose this when:
 * - Statistical quality is paramount
 * - Memory constraints are tight
 * - You need guaranteed independent streams
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
