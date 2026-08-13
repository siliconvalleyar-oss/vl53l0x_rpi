# Coding Rules and Conventions

## Language Standards

- **C++ Standard:** C++17
- **Compiler:** g++ >= 7 or clang++ >= 5
- **Standard Library:** Use STL whenever possible

## Naming Conventions

### Files
- Headers: `PascalCase.hpp`
- Source: `PascalCase.cpp`
- Scripts: `snake_case.sh`

### Classes and Structs
- `PascalCase` (e.g., `Vl53l0x_t`, `I2cBus`)

### Functions and Methods
- `snake_case` (e.g., `init()`, `medir()`, `calibrar_offset()`)

### Variables
- `snake_case` for local variables (e.g., `distance`, `i2c_address`)
- `snake_case` for member variables (e.g., `offset_`, `is_initialized_`)
- Prefix private members with underscore suffix

### Constants
- `UPPER_SNAKE_CASE` (e.g., `VL53L0X_MAX_DISTANCE`, `I2C_DEFAULT_ADDRESS`)

### Namespaces
- `PascalCase` (e.g., `VL53L0X`, `OLED`, `UTILS`)

### Enums and Enumerators
- Enum type: `PascalCase` (e.g., `MeasurementMode`)
- Enumerators: `PascalCase` (e.g., `SINGLE_SHOT`, `CONTINUOUS`)

## Code Style

### Braces
- Opening brace on same line for functions/classes
- Opening brace on new line for control structures (optional)
- Always use braces for single-line blocks

### Indentation
- 4 spaces per indentation level
- No tabs

### Line Length
- Maximum 80 characters per line
- Exceptions: comments, URLs, string literals

### Headers
- Use `#pragma once` for include guards
- No manual `#ifndef` guards
- Include order:
  1. Standard library headers
  2. Third-party library headers
  3. Project headers

### Comments
- Doxygen-style comments for public APIs
- Implementation comments for complex logic
- No commented-out code
- No redundant comments (self-documenting code)

## Memory Management

- **Smart pointers:** Use `std::unique_ptr` for exclusive ownership
- **No raw new/delete:** Never use `new` or `delete` directly
- **RAII:** Acquire resources in constructor, release in destructor
- **Pimpl Idiom:** Use for ABI stability and compilation speed

## Error Handling

- Use return codes for recoverable errors
- Use exceptions for programming errors
- Use `std::optional` for nullable return values
- Always check return values from bcm2835 functions

## Includes

```cpp
// Standard library
#include <memory>
#include <string>
#include <vector>

// Third-party
#include <bcm2835.h>

// Project headers
#include "vl53l0x/VL53L0X.hpp"
#include "oled/SSD1306.hpp"
```

## Const Correctness

- Mark member functions as `const` when they don't modify state
- Use `const` references for input parameters
- Use `const` for member variables that don't change

## Modern C++ Features

- Use `auto` for type inference when type is obvious
- Use range-based for loops
- Use `nullptr` instead of `NULL`
- Use `enum class` instead of plain enums
- Use `override` and `final` keywords
- Use structured bindings when appropriate

## Commit Messages

Follow semantic commit convention:
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation changes
- `style:` Code style changes (formatting, etc.)
- `refactor:` Code refactoring
- `test:` Adding or updating tests
- `chore:` Maintenance tasks

Example: `feat(vl53l0x): add calibration offset support`
