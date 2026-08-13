# Testing Guide

## Test Strategy

### Unit Tests
- Individual class methods
- I2C register read/write operations
- Calibration calculations
- Error handling

### Integration Tests
- Complete sensor initialization
- Measurement accuracy
- Continuous operation stability
- Low power mode transitions

### Hardware Tests
- Real sensor measurements
- Calibration accuracy
- Long-run stability

## Test Environment

### Required Hardware
- Raspberry Pi 3, 4, or 5
- VL53L0X sensor module
- I2C connections (SDA, SCL, VCC, GND)
- Optional: Oscilloscope for signal verification

### Required Software
- bcm2835 library installed
- i2c-tools for bus scanning
- g++ with C++17 support

## Running Tests

### Build Tests

```bash
# Build with test flags
make CXXFLAGS="-g -DTEST_MODE"

# Run unit tests
./bin/laser_measure_test
```

### Manual Test Procedure

#### Test 1: I2C Communication

```bash
# Verify sensor is detected
sudo i2cdetect -y 1
# Should show 29 at address 0x29
```

**Expected:** Address 29 appears in scan

#### Test 2: Initialization

```cpp
auto sensor = std::make_unique<VL53L0X::Vl53l0x_t>();
bool result = sensor->init();
```

**Expected:** Returns true

#### Test 3: Single Measurement

```cpp
uint16_t distance = sensor->medir();
```

**Expected:** Distance between 30mm and 2000mm

#### Test 4: Calibration

```cpp
// Place object at known distance (e.g., 100mm)
sensor->calibrar(0);  // Reset offset
uint16_t reading = sensor->medir();

// Adjust offset until reading matches known distance
int16_t offset = 100 - reading;
sensor->calibrar(offset);

// Verify
reading = sensor->medir();  // Should be ~100mm
```

**Expected:** Reading within ±5% of known distance

#### Test 5: Continuous Operation

```bash
sudo ./bin/laser_measure
```

Let run for 10 minutes, observe:
- No crashes
- Stable readings
- No memory leaks (check with `top`)

**Expected:** Stable continuous output

#### Test 6: Low Power Modes

```cpp
sensor->sleep();
// Wait 5 seconds
sensor->wake();
uint16_t distance = sensor->medir();
```

**Expected:** Measurement works after wake

#### Test 7: Signal Handling

```bash
sudo ./bin/laser_measure
# Press Ctrl+C
```

**Expected:**
```
^C
Saliendo del programa... Liberando recursos.
Memoria liberada correctamente.
```

## Automated Testing

### Test Script

```bash
#!/bin/bash
# scripts/test.sh

echo "Running VL53L0X tests..."

# Test 1: I2C detection
echo "Test 1: I2C device detection"
if sudo i2cdetect -y 1 | grep -q "29"; then
    echo "  PASS: Device found at 0x29"
else
    echo "  FAIL: Device not found"
    exit 1
fi

# Test 2: Build
echo "Test 2: Build project"
./scripts/build.sh
if [ -f "bin/laser_measure" ]; then
    echo "  PASS: Binary created"
else
    echo "  FAIL: Build failed"
    exit 1
fi

# Test 3: Initialization
echo "Test 3: Sensor initialization"
# (Requires hardware)

echo "All tests completed"
```

## Performance Benchmarks

### Expected Performance

| Metric | Target | Minimum |
|--------|--------|---------|
| Measurement time | 20ms | 50ms |
| I2C throughput | 100 kB/s | 50 kB/s |
| Memory usage | < 1MB | < 5MB |
| CPU usage (idle) | < 1% | < 5% |

### Benchmarking

```bash
# Measure execution time
time sudo ./bin/laser_measure -n 100

# Monitor CPU usage
top -p $(pgrep laser_measure)

# Check memory
ps aux | grep laser_measure
```

## Continuous Integration

### GitHub Actions (Future)

```yaml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install -y libbcm2835-dev
      - name: Build
        run: make
      - name: Run tests
        run: make test
```

## Test Coverage

Current coverage goals:
- I2C operations: 100%
- Initialization: 100%
- Measurement logic: 90%
- Error handling: 85%
- Overall: > 80%

## Known Test Limitations

- Hardware-dependent tests cannot run in CI
- I2C mocking not yet implemented
- Timing-sensitive tests may be flaky
