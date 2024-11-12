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

#ifndef BPR_ENGINE_HPP
#define BPR_ENGINE_HPP

#include <cstdint>
#include <array>

namespace bpr {

template <typename T, size_t N>
class IEngine
{
public:
    constexpr IEngine(std::array<T, N>&& state) noexcept
        : m_state(std::move(state))
    { }

    virtual ~IEngine() = default;
    virtual uint64_t next() noexcept = 0;

public:
    using StateValueType = T;
    static constexpr size_t STATE_SIZE = N;

protected:
    std::array<T, N> m_state;
};

template<typename EngineType>
struct EngineTraits {
    using StateValueType = typename EngineType::StateValueType;
    static constexpr size_t STATE_SIZE = EngineType::STATE_SIZE;

    static constexpr bool is_valid_engine = 
        std::is_base_of_v<IEngine<StateValueType, STATE_SIZE>, EngineType>;
};

} // namespace bpr

#endif // BPR_ENGINE_HPP
