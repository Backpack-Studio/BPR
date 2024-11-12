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

#ifndef BPR_CSPRNG_HPP
#define BPR_CSPRNG_HPP

#include "engine.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <random>
#include <array>

namespace bpr { namespace csprng {

/**
 * @brief ChaCha20 Cryptographically Secure Pseudo-Random Number Generator
 * 
 * @details
 * A high-performance CSPRNG based on the ChaCha20 stream cipher algorithm.
 * Recommended for:
 * - High-throughput applications requiring fast random number generation
 * - Applications needing a simple, compact implementation
 * - Cases where a modern, well-analyzed algorithm is preferred
 * 
 * Key features:
 * - 256-bit security strength
 * - Simple and compact implementation
 * - No complex key schedule required
 * - Designed for efficient software implementation
 * - Based on simple 32-bit operations (addition, XOR, rotation)
 * 
 * Performance characteristics:
 * - State size: 64 bytes
 * - Initialization: Fast (no key schedule)
 * - Code size: Compact (simpler operations)
 * - Generation speed: Very good (optimized for software)
 * 
 * @example
 * ```cpp
 * std::random_device rd;
 * ChaCha20 rng(rd);
 * uint64_t random_number = rng.next();          // Generate a 64-bit random number
 * auto block = rng.next512();                   // Generate 512 bits of random data
 * ```
 * 
 * @see RFC 8439: ChaCha20 and Poly1305 for IETF Protocols
 */
class ChaCha20 : public IEngine<uint32_t, 16>
{
public:
    explicit ChaCha20(std::random_device& rd)
        : IEngine({})
    {
        // Constants "expand 32-byte k"
        for (int i = 0; i < 4; i++) {
            m_state[i] = EXPAND_32_BYTE_K[i];
        }
        
        // Key (use of seed)
        // We divide the seed into 4 32-bit words
        for (int i = 0; i < 2; ++i) {
            m_state[4 + i*2] = rd();
            m_state[5 + i*2] = rd();
        }

        // Counter (starts at 0)
        m_state[12] = 0;
        m_state[13] = 0;
        
        // Nonce (divided into 2 32-bit words)
        m_state[14] = rd();
        m_state[15] = rd();
    }

    ChaCha20(const std::array<uint32_t, 8>& key, const std::array<uint32_t, 2>& nonce) noexcept
        : IEngine({})
    {
        // Constants "expand 32-byte k"
        for (int i = 0; i < 4; i++) {
            m_state[i] = EXPAND_32_BYTE_K[i];
        }

        // Key (using user-provided key)
        for (int i = 0; i < 8; ++i) {
            m_state[4 + i] = key[i];
        }

        // Counter (starts at 0)
        m_state[12] = 0;
        m_state[13] = 0;

        // Nonce (using user-provided nonce)
        m_state[14] = nonce[0];
        m_state[15] = nonce[1];
    }

    uint64_t next() noexcept override {
        std::array<uint32_t, 16> result = block();
        uint64_t combined = 0;
        for (size_t i = 0; i < 16; i += 2) {
            combined ^= (static_cast<uint64_t>(result[i]) << 32) | result[i + 1];
        }
        return combined;
    }

    std::array<uint32_t, 16> next512() noexcept {
        return block();
    }

private:
    static constexpr std::array<uint32_t, 4> EXPAND_32_BYTE_K
    {
        0x61707865,
        0x3320646e,
        0x79622d32,
        0x6b206574
    };

private:
    static void quarter_round(uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) noexcept {
        *a += *b; *d ^= *a; *d = rotl(*d, 16);
        *c += *d; *b ^= *c; *b = rotl(*b, 12);
        *a += *b; *d ^= *a; *d = rotl(*d, 8);
        *c += *d; *b ^= *c; *b = rotl(*b, 7);
    }

    void round(std::array<uint32_t, 16>& working_state) noexcept {
        // Columns
        quarter_round(&working_state[0], &working_state[4], &working_state[8], &working_state[12]);
        quarter_round(&working_state[1], &working_state[5], &working_state[9], &working_state[13]);
        quarter_round(&working_state[2], &working_state[6], &working_state[10], &working_state[14]);
        quarter_round(&working_state[3], &working_state[7], &working_state[11], &working_state[15]);
        // Diagonals
        quarter_round(&working_state[0], &working_state[5], &working_state[10], &working_state[15]);
        quarter_round(&working_state[1], &working_state[6], &working_state[11], &working_state[12]);
        quarter_round(&working_state[2], &working_state[7], &working_state[8], &working_state[13]);
        quarter_round(&working_state[3], &working_state[4], &working_state[9], &working_state[14]);
    }

    std::array<uint32_t, 16> block() noexcept {
        std::array<uint32_t, 16> working_state = m_state;
        // 20 rounds (10 double rounds)
        for (int i = 0; i < 10; ++i) {
            round(working_state);
        }
        // Final addition with initial state
        for (auto i = 0; i < STATE_SIZE; ++i) {
            working_state[i] += m_state[i];
        }
        // Counter increment
        m_state[12]++;
        if (m_state[12] == 0) m_state[13]++;
        // Return the block
        return working_state;
    }
};

/**
 * @brief AES-CTR Cryptographically Secure Pseudo-Random Number Generator
 * 
 * @details
 * A software-based CSPRNG implementation using AES in Counter Mode.
 * Recommended for:
 * - Applications requiring a widely standardized algorithm
 * - Environments where AES is a compliance requirement
 * 
 * Key features:
 * - 128-bit security strength
 * - Based on the widely-studied AES block cipher
 * - NIST standardized algorithm
 * - Traditional S-box based design
 * 
 * Performance characteristics:
 * - State size: 192 bytes (includes expanded key)
 * - Initialization: Moderate (requires key schedule computation)
 * - Code size: Larger (includes S-box and key expansion)
 * - Generation speed: Good for a software AES implementation
 * 
 * @note Implementation characteristics:
 * - Pure software implementation using traditional AES operations
 * - Requires storing expanded key schedule
 * - Uses lookup tables (S-box)
 * 
 * @example
 * ```cpp
 * std::random_device rd;
 * AESCTR rng(rd);
 * uint64_t random_number = rng.next();          // Generate a 64-bit random number
 * auto block = rng.next128();                   // Generate 128 bits of random data
 * ```
 * 
 * @see NIST SP 800-90A: Recommendation for Random Number Generation
 */
class AESCTR : public IEngine<uint8_t, 16>
{
public:
    explicit AESCTR(std::random_device& rd)
        : IEngine({})
    {
        // Here we use 'm_state' as counter
        // Initialize the counter with a random nonce
        for (size_t i = 0; i < m_state.size(); i += sizeof(uint32_t)) {
            uint32_t random = rd();
            memcpy(&m_state[i], &random, sizeof(uint32_t));
        }

        // Generating a random key and expanding the key
        key_expansion(generate_key(rd));
    }

    AESCTR(const std::array<uint8_t, 16>& key, const std::array<uint8_t, 16>& nonce) noexcept
        : IEngine({})
    {
        // Here we use 'm_state' as counter
        // Initializes the counter with a user-supplied nonce
        std::copy(nonce.begin(), nonce.end(), m_state.data());

        // User-supplied key expansion
        key_expansion(key);
    }

    uint64_t next() noexcept override {
        // Calling the process_block function to fill the buffer with 16 bytes of data
        std::array<uint8_t, 16> buffer;
        process_block(buffer);
        // Combines the 16 bytes of the buffer into two uint64_t (high and low)
        uint64_t high = 0, low = 0;
        for (size_t i = 0; i < 8; ++i) {
            // Assemble the first 8 bytes into a uint64_t for the 'high' part
            high |= static_cast<uint64_t>(buffer[i]) << (i * 8);
            // Assemble the next 8 bytes into a uint64_t for the 'low' part
            low |= static_cast<uint64_t>(buffer[i + 8]) << (i * 8);
        }
        // XOR both uint64_t (high and low) to get a 64-bit random number
        return high ^ low;
    }

    std::array<uint64_t, 2> next128() noexcept {
        // Calling the process_block function to fill the buffer with 16 bytes of data
        std::array<uint8_t, 16> buffer;
        process_block(buffer);
        // Copy the 16 bytes of the buffer into the two uint64_t of 'result'
        std::array<uint64_t, 2> result;
        memcpy(result.data(), buffer.data(), 2 * sizeof(uint64_t));
        // Returns the array containing the two uint64_t
        return result;
    }

private:
    static constexpr size_t Nb = 4;  // Nombre de colonnes (32-bit words) dans l'état
    static constexpr size_t Nk = 4;  // Nombre de mots de 32 bits dans la clé
    static constexpr size_t Nr = 10; // Nombre de rounds

    std::array<uint8_t, 176> m_expanded_key;

    static constexpr uint8_t SBOX[256] =
    {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

    static std::array<uint8_t, 16> generate_key(std::random_device& rd) {
        uint32_t words[4];
        for (auto& word : words) {
            word = rd();
        }
        std::array<uint8_t, 16> key;
        for (int i = 0; i < 4; ++i) {
            key[i * 4 + 0] = (words[i] >> 24) & 0xFF;
            key[i * 4 + 1] = (words[i] >> 16) & 0xFF;
            key[i * 4 + 2] = (words[i] >> 8)  & 0xFF;
            key[i * 4 + 3] =  words[i]        & 0xFF;
        }
        return key;
    }

    void key_expansion(const std::array<uint8_t, 16>& key) noexcept {
        // Copy of initial key
        std::copy(key.begin(), key.end(), m_expanded_key.begin());
        // Key expansion
        for (size_t i = Nk; i < Nb * (Nr + 1); ++i) {
            uint32_t temp = *reinterpret_cast<uint32_t*>(&m_expanded_key[4 * (i - 1)]);
            if (i % Nk == 0) {
                // Rotation and substitution
                temp = ((temp << 8) | (temp >> 24));
                uint8_t* temp_bytes = reinterpret_cast<uint8_t*>(&temp);
                for (int j = 0; j < 4; ++j) {
                    temp_bytes[j] = SBOX[temp_bytes[j]];
                }
                // XOR with the round constant
                temp ^= (0x01 << ((i / Nk - 1) * 8));
            }
            *reinterpret_cast<uint32_t*>(&m_expanded_key[4 * i]) = 
                *reinterpret_cast<uint32_t*>(&m_expanded_key[4 * (i - Nk)]) ^ temp;
        }
    }

    /**
     * @brief Increments the 128-bit counter used in the AES CTR mode.
     * 
     * This function is responsible for incrementing the counter that is part of the state used 
     * in the AES Counter (CTR) mode. The counter is a 128-bit value (divided into four 32-bit 
     * words), and it is incremented in a manner that mimics a carry operation, ensuring that 
     * each counter value is unique and increasing monotonically.
     * 
     * The counter is incremented as follows:
     * - The least significant 32-bit word (`counter[3]`) is incremented first.
     * - If this word overflows (i.e., it reaches 0), the next word (`counter[2]`) is incremented.
     * - The overflow propagation continues up to the most significant word (`counter[0]`).
     * 
     * This ensures that the counter rolls over properly, with the least significant part of the
     * counter being incremented first, and the overflow propagating through the higher words if necessary.
     * 
     * The counter plays a critical role in ensuring that each AES block processed in CTR mode is unique, 
     * which is crucial for generating non-repeating pseudo-random numbers.
     * 
     * @note This function modifies the `m_state` array, which is assumed to hold the current 128-bit counter 
     *       value. It assumes that the state array is correctly initialized before calling this function.
     * 
     * @see process_block() for how the counter is used in generating pseudo-random numbers.
     */
    void increment_counter() noexcept {
        // Interpret m_state as an array of 32-bit words to manipulate the counter parts
        uint32_t* counter = reinterpret_cast<uint32_t*>(m_state.data());
        
        // Increment the least significant 32-bit word (counter[3])
        if (++counter[3] == 0) {
            // If counter[3] overflows, increment counter[2]
            if (++counter[2] == 0) {
                // If counter[2] overflows, increment counter[1]
                if (++counter[1] == 0) {
                    // If counter[1] overflows, increment counter[0]
                    ++counter[0];
                }
            }
        }
    }

    /**
     * @brief Processes a 128-bit block using AES operations to generate a pseudo-random value.
     * 
     * This method applies AES transformations on a 128-bit block of state data in the context of 
     * a Counter (CTR) mode generator. The state is XORed with an expanded AES key, and the 
     * byte-wise transformations (`SubBytes` and `ShiftRows`) are applied. Notably, the `MixColumns` 
     * step is omitted here because it is not required in the Counter mode for a random number generator.
     * 
     * The AES transformations are performed in the following order:
     * - **AddRoundKey**: XOR the state with the expanded key.
     * - **SubBytes**: Each byte in the state is substituted using the AES S-Box.
     * - **ShiftRows**: The rows of the state matrix are shifted cyclically (AES-specific).
     * - **AddRoundKey**: Another XOR operation with the expanded key.
     * 
     * After these operations, the counter is incremented to ensure the next block is unique.
     * The `MixColumns` step is intentionally skipped in this function because it is not needed 
     * for the CTR mode where we are simply generating pseudo-random numbers.
     * 
     * @param state The 128-bit block of state data to be processed. This data will be transformed 
     *              in-place, with the processed result stored in the same array.
     * 
     * @note This method is designed specifically for Counter mode and should not be used for 
     *       traditional AES encryption, where `MixColumns` is an essential part of the process.
     */
    void process_block(std::array<uint8_t, 16>& state) noexcept {
        // Initial AddRoundKey: XOR the state with the first part of the expanded key
        for (size_t i = 0; i < 16; ++i) {
            state[i] ^= m_expanded_key[i];
        }

        // SubBytes: Permute each byte using the AES S-box
        for (auto& byte : state) {
            byte = SBOX[byte];
        }

        // ShiftRows: Perform the row shifts as defined in AES
        std::array<uint8_t, 16> temp = state;
        state[1] = temp[5];
        state[5] = temp[9];
        state[9] = temp[13];
        state[13] = temp[1];
        state[2] = temp[10];
        state[6] = temp[14];
        state[10] = temp[2];
        state[14] = temp[6];
        state[3] = temp[15];
        state[7] = temp[3];
        state[11] = temp[7];
        state[15] = temp[11];

        // AddRoundKey: XOR the state with the second part of the expanded key
        for (size_t i = 0; i < 16; ++i) {
            state[i] ^= m_expanded_key[16 + i];
        }

        // Increment the counter to ensure the next block is unique
        increment_counter();
    }
};

}} // namespace bpr::csprng

#endif // BPR_CSPRNG_HPP
