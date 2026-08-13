# Architecture Documentation

## System Overview

The project follows a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────┐
│           Application Layer                  │
│         (main.cpp - main loop)              │
├─────────────────────────────────────────────┤
│           Sensor Abstraction Layer          │
│    (VL53L0X::Vl53l0x_t - public API)       │
├─────────────────────────────────────────────┤
│           Hardware Abstraction Layer        │
│    (bcm2835 wrapper, I2C operations)       │
├─────────────────────────────────────────────┤
│           Physical Layer                    │
│    (Raspberry Pi GPIO, I2C bus 1)          │
└─────────────────────────────────────────────┘
```

## Class Diagram

```
┌─────────────────────────────────────────────────────┐
│                  Vl53l0x_t                          │
├─────────────────────────────────────────────────────┤
│ - i2c_address: uint8_t                             │
│ - offset: int16_t                                  │
│ - is_initialized: bool                            │
│ - measurement_mode: MeasurementMode                │
│ - pimpl: std::unique_ptr<Impl>                     │
├─────────────────────────────────────────────────────┤
│ + Vl53l0x_t()                                      │
│ + ~Vl53l0x_t()                                     │
│ + init(): bool                                     │
│ + medir(): uint16_t                                │
│ + calibrar(offset: int16_t): void                  │
│ + sleep(): void                                    │
│ + wake(): void                                     │
│ + set_measurement_mode(mode: MeasurementMode): void│
└─────────────────────────────────────────────────────┘
```

## Data Flow

### Measurement Flow

1. **Application** calls `laser->medir()`
2. **VL53L0X Layer** validates state and prepares measurement
3. **Hardware Layer** writes to VL53L0X registers via I2C
4. **Sensor** performs time-of-flight measurement
5. **Hardware Layer** reads result registers via I2C
6. **VL53L0X Layer** applies calibration offset
7. **Result** returned to application as uint16_t (mm)

### Initialization Flow

1. **bcm2835_i2c_begin()** - Initialize I2C peripheral
2. **bcm2835_i2c_setSlaveAddress(0x29)** - Set sensor address
3. **Read device ID** from register 0xC0
4. **Verify ID** equals 0xEE (VL53L0X ID)
5. **Configure** measurement parameters:
   - VHV calibration
   - Phase calibration
   - Measurement timing budget
6. **Set continuous measurement mode**

## Memory Management

All resources managed via RAII:
- `std::unique_ptr` for exclusive ownership of sensor instances
- Automatic cleanup when objects go out of scope
- No manual `new`/`delete` required
- Exception-safe initialization and cleanup

## Thread Safety

- Single-threaded design (no concurrent access)
- Signal handlers for clean shutdown (SIGINT, SIGTERM)
- Atomic flag for graceful loop termination

## Error Handling

- Return codes for initialization failures
- `std::optional<uint16_t>` for optional measurement results
- Exception handling in main loop
- Detailed error messages via logging
