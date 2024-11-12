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

#ifndef BPR_GENERATOR_HPP
#define BPR_GENERATOR_HPP

#include "engine.hpp"
#include "utils.hpp"

#include <unordered_set>
#include <type_traits>
#include <vector>
#include <cmath>

namespace bpr {

/**
 * @brief Generates a random value of type T using the provided random engine.
 * 
 * This function can generate random values of either integral or floating-point types.
 * It uses the next() method of the provided random engine to generate random numbers.
 * 
 * @tparam T The type of the value to be generated. It can be either integral (e.g., int, uint32_t) or floating-point (e.g., float, double).
 * @tparam Engine The type of the random engine that is used to generate random values. It must be a class that implements a `next()` method.
 * 
 * @param e The random engine used to generate random values. It must be an object that meets the requirements of the Engine concept.
 * 
 * @return A random value of type T.
 * 
 * @throws static_assert If the provided engine type does not meet the requirements of the Engine concept, or if T is not an integral or floating-point type.
 */
template <typename T, typename Engine>
constexpr T rand(Engine& e) noexcept
{
    // Assert that the Engine type is valid (derived from IEngine)
    static_assert(EngineTraits<Engine>::is_valid_engine, "EngineType must be derived from IEngine");

    // If T is an integral type, return the next generated value as the desired type
    if constexpr (std::is_integral_v<T>) {
        return static_cast<T>(e.next());
    }

    // If T is a floating-point type, scale the next generated value to a value between 0 and 1
    else if constexpr (std::is_floating_point_v<T>) {
        // 1.0 / UINT64_MAX is the inverse of the largest possible random value
        constexpr T inv_max = 1.0 / static_cast<T>(UINT64_MAX);
        // Scale the random value to a floating-point value between 0.0 and 1.0
        return static_cast<T>(e.next()) * inv_max;
    }

    // If T is neither integral nor floating-point, trigger a compile-time error
    else {
        static_assert(always_false<T>, "T must be an integer or a floating point");
    }
}

/**
 * @brief Generates a random value of type T in the specified range [min, max] using the provided random engine.
 * 
 * This function can generate random values of either integral or floating-point types.
 * The random value is uniformly distributed within the specified range. The range is inclusive for integral types
 * and is a continuous range for floating-point types.
 * 
 * @tparam T The type of the value to be generated. It can be either integral (e.g., int, uint32_t) or floating-point (e.g., float, double).
 * @tparam Engine The type of the random engine that is used to generate random values. It must be a class that implements a `next()` method.
 * 
 * @param e The random engine used to generate random values. It must be an object that meets the requirements of the Engine concept.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * 
 * @return A random value of type T within the range [min, max].
 * 
 * @throws static_assert If the provided engine type does not meet the requirements of the Engine concept, or if T is not an integral or floating-point type.
 */
template <typename T, typename Engine>
constexpr T rand(Engine& e, T min, T max) noexcept
{
    // Assert that the Engine type is valid (derived from IEngine)
    static_assert(EngineTraits<Engine>::is_valid_engine, "EngineType must be derived from IEngine");

    // If T is an integral type, generate a random number in the range [min, max]
    if constexpr (std::is_integral_v<T>) {
        // Calculate the range size (inclusive of max)
        T range = max - min + 1;
        // Generate a random number in the range [min, max]
        return min + static_cast<T>(e.next()) % range;
    } 

    // If T is a floating-point type, generate a random number in the range [min, max]
    else if constexpr (std::is_floating_point_v<T>) {
        // Calculate the range size (floating-point range)
        T range = max - min;
        // Scale the random value to the range [min, max] using the next() value divided by UINT64_MAX
        return min + static_cast<T>(e.next()) / static_cast<T>(UINT64_MAX) * range;
    }

    // If T is neither integral nor floating-point, trigger a compile-time error
    else {
        static_assert(always_false<T>, "T must be an integer or a floating point");
    }
}

/**
 * @brief Generates a sequence of unique random values within the range [min, max].
 * 
 * This function generates a sequence of `count` unique random values of type T within the specified range [min, max].
 * The values can be either integral or floating-point types, and the function ensures that no duplicate values are added.
 * 
 * If the requested `count` exceeds the number of possible unique values within the range, the function adjusts `count`
 * to the maximum number of unique values that can be generated within the range.
 * 
 * @tparam T The type of the values to be generated. It can be either integral (e.g., int, uint32_t) or floating-point (e.g., float, double).
 * @tparam Engine The type of the random engine used to generate random values. It must be a class that implements a `next()` method.
 * 
 * @param e The random engine used to generate random values. It must meet the requirements of the Engine concept.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @param count The number of unique random values to generate.
 * 
 * @return A vector containing the unique random values.
 * 
 * @throws static_assert If the provided engine type does not meet the requirements of the Engine concept, or if T is not an integral or floating-point type.
 */
template <typename T, typename Engine>
std::vector<T> sequence(Engine& e, T min, T max, size_t count)
{
    // For integral types, ensure that `count` does not exceed the range of unique values
    if constexpr (std::is_integral_v<T>) {
        if (count > static_cast<size_t>(max - min) + 1) {
            count = static_cast<size_t>(max - min + 1);
        }
    } 

    // For floating-point types, calculate the maximum number of unique values based on epsilon
    else if constexpr (std::is_floating_point_v<T>) {
        T range = max - min;
        // Calculate the number of unique values that can be represented within the range
        size_t unique_count = static_cast<size_t>(std::ceil(range / std::numeric_limits<T>::epsilon()));
        if (count > unique_count) count = unique_count;
    }

    // Vector to store the unique random values
    std::vector<T> seq;
    seq.reserve(count);
    std::unordered_set<T> unique_values;

    // Generate values until we have `count` unique values
    while (seq.size() < count) {
        T value = rand(e, min, max);
        // Insert the value into the set; if successful, add it to the sequence
        if (unique_values.insert(value).second) {
            seq.push_back(value);
        }
    }

    return seq;
}

} // namespace bpr

#endif // BPR_GENERATOR_HPP
