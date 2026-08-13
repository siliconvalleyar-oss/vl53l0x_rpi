# API Reference

## VL53L0X::Vl53l0x_t

Main sensor class for VL53L0X time-of-flight laser ranging sensor.

### Constructor

```cpp
Vl53l0x_t();
```

Creates a new VL53L0X sensor instance. Sensor is not initialized until `init()` is called.

### Destructor

```cpp
~Vl53l0x_t();
```

Automatically releases I2C resources via RAII.

### Public Methods

#### init()

```cpp
bool init();
```

Initializes the VL53L0X sensor and I2C communication.

**Returns:**
- `true` - Sensor initialized successfully
- `false` - Initialization failed

**Throws:** None

#### medir()

```cpp
uint16_t medir();
```

Performs a single distance measurement.

**Returns:**
- Distance in millimeters (uint16_t)
- 0 or 65535 on error

**Throws:** None

#### calibrar()

```cpp
void calibrar(int16_t offset);
```

Sets calibration offset for distance measurements.

**Parameters:**
- `offset` - Calibration offset in millimeters (-100 to 100 typical)

**Throws:** None

#### sleep()

```cpp
void sleep();
```

Puts sensor in low-power sleep mode.

**Throws:** None

#### wake()

```cpp
void wake();
```

Wakes sensor from sleep mode.

**Throws:** None

#### set_measurement_mode()

```cpp
void set_measurement_mode(MeasurementMode mode);
```

Sets the measurement mode.

**Parameters:**
- `mode` - MeasurementMode enum value

**MeasurementMode values:**
- `SINGLE_SHOT` - Single measurement
- `CONTINUOUS` - Continuous measurement mode
- `TIMED` - Timed measurement

## Enums

### MeasurementMode

```cpp
enum class MeasurementMode {
    SINGLE_SHOT = 0,
    CONTINUOUS = 1,
    TIMED = 2
};
```

### Vl53l0xError

```cpp
enum class Vl53l0xError {
    NONE = 0,
    I2C_ERROR = 1,
    INVALID_DEVICE = 2,
    TIMEOUT = 3,
    CALIBRATION_ERROR = 4,
    NOT_INITIALIZED = 5
};
```

## Constants

### VL53L0X_DEFAULT_ADDRESS

```cpp
constexpr uint8_t VL53L0X_DEFAULT_ADDRESS = 0x29;
```

Default I2C address of VL53L0X sensor.

### VL53L0X_MAX_DISTANCE

```cpp
constexpr uint16_t VL53L0X_MAX_DISTANCE = 2000;  // mm
```

Maximum measurable distance (2 meters in long range mode).

### VL53L0X_MIN_DISTANCE

```cpp
constexpr uint16_t VL53L0X_MIN_DISTANCE = 30;    // mm
```

Minimum measurable distance.

## Global Functions

### get_error_string()

```cpp
const char* get_error_string(Vl53l0xError error);
```

Returns human-readable error message.

## Structs

### Vl53l0xConfig

```cpp
struct Vl53l0xConfig {
    uint16_t timing_budget_ms;
    uint16_t inter_measurement_period_ms;
    uint8_t vhv_config;
    bool continuous_mode;
};
```

Configuration structure for sensor initialization.

## Usage Examples

### Basic Measurement

```cpp
#include "vl53l0x/VL53L0X.hpp"

int main() {
    auto sensor = std::make_unique<VL53L0X::Vl53l0x_t>();
    
    if (sensor->init()) {
        uint16_t distance = sensor->medir();
        std::cout << "Distance: " << distance << " mm" << std::endl;
    }
    
    return 0;
}
```

### Continuous Measurement

```cpp
auto sensor = std::make_unique<VL53L0X::Vl53l0x_t>();
sensor->init();

while (running) {
    uint16_t distance = sensor->medir();
    std::cout << "Distance: " << distance << " mm" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
```

### Calibration

```cpp
sensor->calibrar(10);  // Add 10mm offset
uint16_t calibrated_distance = sensor->medir();
```
