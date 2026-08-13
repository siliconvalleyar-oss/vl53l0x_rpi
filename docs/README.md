# VL53L0X Laser Distance Sensor - Raspberry Pi

Professional C++17 project for VL53L0X time-of-flight laser ranging sensor using bcm2835 library on Raspberry Pi.

## Features

- Complete VL53L0X sensor wrapper using bcm2835 I2C
- Smart pointer memory management (RAII)
- Continuous distance measurement mode
- Calibration offset support
- Low power sleep/wake modes
- Cross-architecture support (32-bit armv7l, 64-bit aarch64)
- Clean shutdown with signal handling
- Comprehensive documentation and examples

## Requirements

- Raspberry Pi 3, 4, or 5
- Raspberry Pi OS (32-bit or 64-bit)
- bcm2835 library
- C++17 compatible compiler (g++ >= 7)
- Sudo privileges (required for bcm2835 GPIO/I2C access)

## Quick Start

```bash
# Install dependencies
sudo ./scripts/install_deps.sh

# Build project
./scripts/build.sh

# Run sensor
sudo ./scripts/run.sh
```

## Project Structure

```
├── src/           # Source code
│   ├── main.cpp
│   ├── vl53l0x/   # VL53L0X sensor implementation
│   └── oled/      # OLED display support (optional)
├── include/       # Header files
│   ├── vl53l0x/   # VL53L0X headers
│   ├── oled/      # OLED headers
│   └── lib/       # Utility headers
├── docs/          # Documentation
├── scripts/       # Build and utility scripts
├── bin/           # Compiled binaries
└── obj/           # Object files
```

## Usage Example

```bash
$ sudo ./bin/laser_measure
Iniciando sensor VL53L0X...
Sensor inicializado correctamente.
Distancia: 245 mm
Distancia: 250 mm
Distancia: 242 mm
^C
Saliendo del programa... Liberando recursos.
Memoria liberada correctamente.
```

## Hardware Connection

| VL53L0X Pin | Raspberry Pi GPIO | Description |
|-------------|-------------------|-------------|
| VIN         | 3.3V (Pin 1)      | Power |
| GND         | GND (Pin 6)       | Ground |
| SDA         | GPIO2 (Pin 3)     | I2C Data |
| SCL         | GPIO3 (Pin 5)     | I2C Clock |

I2C address: `0x29` (default)

## Documentation

- [Architecture](ARCHITECTURE.md) - System design and data flow
- [API Reference](API.md) - Complete API documentation
- [Setup Guide](SETUP.md) - Raspberry Pi initial configuration
- [Testing Guide](TESTING.md) - Unit and integration tests
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues and solutions
- [Coding Rules](RULES.md) - Code style and conventions
- [Todo List](TODO.md) - Planned features and improvements
- [Changelog](CHANGELOG.md) - Version history

## License

MIT License - See LICENSE file for details.

## Author

Professional embedded C++ project for Raspberry Pi.
