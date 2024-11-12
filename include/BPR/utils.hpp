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

#ifndef BPR_UTILS_HPP
#define BPR_UTILS_HPP

#include <type_traits>
#include <cstdint>

namespace bpr {

/**
 * @brief A compile-time constant that is always false for any type T.
 * 
 * This constexpr variable is used as a static assertion helper to trigger a compilation error
 * when an invalid type is passed to a function.
 * 
 * It is used in `rotl()` and other functions to ensure that the type is compatible with the operations being performed.
 * 
 * @tparam T The type of the value. This can be any type, and is always set to false at compile-time.
 */
template <typename T>
constexpr bool always_false = static_cast<T>(0);

/**
 * @brief A helper function that converts the current time (from the `__TIME__` macro) into a compile-time integer.
 * 
 * This function takes advantage of the `__TIME__` predefined macro, which contains the current time of compilation in the format "HH:MM:SS".
 * It converts this string representation of time into a total number of seconds since midnight, providing a unique value at compile-time.
 * The return value is used as the default seed for the PRNG generation engines in this library.
 * 
 * @return A 64-bit integer representing the number of seconds since midnight.
 */
constexpr uint64_t compile_time() noexcept
{
    return ((uint64_t)((10 * (__TIME__[0] - '0')) + (__TIME__[1] - '0'))) * 3600 +  // Hours to seconds
           ((uint64_t)((10 * (__TIME__[3] - '0')) + (__TIME__[4] - '0'))) * 60 +    // Minutes to seconds
           ((uint64_t)((10 * (__TIME__[6] - '0')) + (__TIME__[7] - '0')));          // Seconds
}

/**
 * @brief A left-rotation (bitwise rotation) function for integer types.
 * 
 * This function performs a left rotation on the given value `x` by `k` positions.
 * The behavior of the function depends on the type of `T`:
 * - If `T` is `uint64_t`, it rotates the value by `k` positions in a 64-bit register.
 * - If `T` is `uint32_t`, it rotates the value by `k` positions in a 32-bit register.
 * 
 * A static assertion is used to prevent the use of unsupported types.
 * 
 * @tparam T The type of the value to rotate. Must be either `uint64_t` or `uint32_t`.
 * @param x The value to rotate.
 * @param k The number of positions to rotate the value to the left.
 * @return The result of rotating the value `x` by `k` positions.
 * 
 * @throws static_assert If `T` is not a supported type (i.e., not `uint64_t` or `uint32_t`).
 */
template <typename T>
constexpr T rotl(const T x, int k) noexcept
{
    // If T is uint64_t, perform the rotation in a 64-bit register
    if constexpr (std::is_same_v<T, uint64_t>) {
        return (x << k) | (x >> (64 - k));
    }

    // If T is uint32_t, perform the rotation in a 32-bit register
    else if constexpr (std::is_same_v<T, uint32_t>) {
        return (x << k) | (x >> (32 - k));
    }

    // Static assertion to ensure that T is a valid type for the operation
    else {
        static_assert(always_false<T>, "T is incompatible");
    }
}

/**
 * @brief A fast 64-bit PRNG (pseudo-random number generator) algorithm, SplitMix64.
 * 
 * This function implements the SplitMix64 algorithm, a high-quality, fast, and statistically good PRNG.
 * 
 * The algorithm performs a series of bitwise operations on the seed value:
 * - The seed is incremented by a constant value (`0x9e3779b97f4a7c15`), known as the "golden ratio".
 * - Several shifts, XORs, and multiplications are applied to mix the bits.
 * - The result is returned as the next value in the sequence.
 * 
 * @param seed The initial seed value used to generate the random number.
 * @return A 64-bit pseudo-random number.
 */
constexpr uint64_t splitmix64(uint64_t seed) noexcept
{
    uint64_t z = (seed += 0x9e3779b97f4a7c15);  // Add the golden ratio to the seed
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;   // Mix the bits with XOR and a large prime constant
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;   // Further mixing with another prime constant
    return z ^ (z >> 31);                       // Final mixing step, returning the result
}

} // namespace bpr

#endif // BPR_UTILS_HPP
