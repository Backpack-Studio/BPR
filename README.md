# Backpack Random (BPR)

**Backpack Random (BPR)** is a lightweight, header-only C++ library for generating random numbers, part of the Backpack ecosystem. BPR provides both pseudorandom (PRNG) and cryptographically secure random number generators (CSPRNG).

## Features

- **PRNG Implementations**:
  - `Xoroshiro128+`
  - `Xoroshiro128++`
  - `Xoroshiro128**`
  - `Xoshiro256+`
  - `Xoshiro256++`
  - `Xoshiro256**`
  - `PCG32`

- **CSPRNG Implementations**:
  - `ChaCha20`
  - `AES-CTR`

- **Header-only Library**: Compatible with C++17 and above
- **Typed Random Generators**: Generate values of any integral or floating-point type.
- **Virtual Interface**: All generators inherit from the `IEngine` interface, ensuring a consistent API and extensibility.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Installation](#installation)
3. [Usage](#usage)
    - [Basic Example](#basic-example)
    - [Generating Values](#generating-values)
    - [Range-Based Values](#range-based-values)
    - [Unique Random Sequences](#unique-random-sequences)
4. [API Reference](#api-reference)
5. [License](#license)

---

## Getting Started

BPR provides several random number engines which can be used independently. Each engine inherits from the `IEngine` interface, requiring a `next()` method to produce random numbers. 

### Project Structure
The current structure of BPR is as follows:

```
├───examples
│       example.cpp
│
└───include
    └───BPR
            BPR.hpp
            csprng.hpp
            engine.hpp
            generator.hpp
            prng.hpp
            utils.hpp
```

## Installation

BPR is header-only and does not require building. To integrate BPR into your project:

1. Clone the repository:
    ```bash
    git clone https://github.com/Backpack-Studio/BPR.git
    ```
2. Include the BPR headers in your project:
    ```cpp
    #include <BPR/BPR.hpp>
    ```
3. Compile your project with C++17 or higher.

## Usage

### Basic Example
Here's a basic example using the `Xoshiro256**` PRNG to generate a sequence of random numbers:

```cpp
#include <BPR/BPR.hpp>
#include <iostream>

int main() {
    bpr::prng::Xoshiro256ss engine;
    auto sequence = bpr::sequence(engine, 0.1, 0.2, 1000);
    for (auto value : sequence) {
        std::cout << value << std::endl;
    }
}
```

### Generating Values

To generate a single random number of a specific type:

```cpp
#include <BPR/BPR.hpp>

int main() {
    bpr::prng::Xorshift128p engine;
    int random_int = bpr::rand<int>(engine);                // Float in range [int min, int max]
    double random_double = bpr::rand<double>(engine);       // Float in range [0.0, 1.0]

    // Output the results
    std::cout << "Random Integer: " << random_int << std::endl;
    std::cout << "Random Double: " << random_double << std::endl;
}
```

### Range-Based Values

To generate values within a specific range:

```cpp
#include <BPR/BPR.hpp>
#include <iostream>

int main() {
    bpr::prng::PCG32 engine;
    int bounded_int = bpr::rand(engine, 10, 50);            // Integer in range [10, 50]
    float bounded_float = bpr::rand(engine, 0.0f, 5.0f);    // Float in range [0.0, 5.0]

    std::cout << "Bounded Integer: " << bounded_int << std::endl;
    std::cout << "Bounded Float: " << bounded_float << std::endl;
}
```

### Unique Random Sequences

Generate a sequence of unique values within a specified range:

```cpp
#include <BPR/BPR.hpp>
#include <iostream>
#include <vector>

int main() {
    bpr::csprng::ChaCha20 engine;
    auto unique_sequence = bpr::sequence<int>(engine, 1, 100, 10);

    std::cout << "Unique Sequence: ";
    for (const auto& value : unique_sequence) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}
```

## API Reference

### `rand` Function

- **Description**: Generates a random value of type `T` using a specified random engine.
- **Syntax**:
  ```cpp
  template <typename T, typename Engine>
  T rand(Engine& engine);
  ```
- **Parameters**:
  - `T`: Type of the value to generate. Supports `int`, `float`, `double`, etc.
  - `Engine`: Random engine to use for generation (must implement `next()`).
- **Returns**: A random value of type `T`.

### `rand` with Range

- **Description**: Generates a random value within a specified range.
- **Syntax**:
  ```cpp
  template <typename T, typename Engine>
  T rand(Engine& engine, T min, T max);
  ```
- **Parameters**:
  - `min`: Minimum bound of the range.
  - `max`: Maximum bound of the range.
- **Returns**: A random value in the range `[min, max]`.

### `sequence` Function

- **Description**: Generates a sequence of unique random values within a specified range.
- **Syntax**:
  ```cpp
  template <typename T, typename Engine>
  std::vector<T> sequence(Engine& engine, T min, T max, size_t count);
  ```
- **Parameters**:
  - `min`, `max`: The range in which values are generated.
  - `count`: The number of unique random values to generate.
- **Returns**: A vector containing the generated values.

## License

This library is provided under the **zlib License**. See the [LICENSE](LICENSE) file for full details.
