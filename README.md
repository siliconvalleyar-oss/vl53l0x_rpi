# VL53L0X Laser Distance Sensor - Raspberry Pi

Professional C++17 project for VL53L0X time-of-flight laser ranging sensor using bcm2835 library on Raspberry Pi.

## Features

- Complete VL53L0X sensor wrapper using the Linux kernel I2C driver (`/dev/i2c-1`) and bcm2835 for GPIO
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
- Sudo privileges (required for bcm2835 GPIO access)

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

| VL53L0X Pin | Raspberry Pi GPIO | Header Pin | Configuration in code |
|-------------|-------------------|------------|------------------------|
| VIN         | 3.3 V             | Pin 1      | Power (module supports 3.3–5 V) |
| GND         | GND               | Pin 6      | Ground |
| SDA         | GPIO2             | Pin 3      | I2C data — kernel bus `/dev/i2c-1` (100 kHz) |
| SCL         | GPIO3             | Pin 5      | I2C clock — kernel bus `/dev/i2c-1` (100 kHz) |
| XSHUT       | GPIO17            | Pin 11     | Output: reset/wake del módulo (LOW 10 ms → HIGH 10 ms al iniciar) |
| GPIO1 (INT) | GPIO27            | Pin 13     | Input: línea de interrupción del sensor (no usada en el modo actual) |

I2C address: `0x29` (default)

Configuración de pines en el código (`src/main.cpp`):

```cpp
laser->set_xshut_pin(17);   // GPIO17  -> XSHUT (salida, pulso de reset)
laser->set_gpio1_pin(27);   // GPIO27  -> GPIO1/INT (entrada)
```

## Documentation

- [Architecture](docs/ARCHITECTURE.md) - System design and data flow
- [API Reference](docs/API.md) - Complete API documentation
- [Setup Guide](docs/SETUP.md) - Raspberry Pi initial configuration
- [Testing Guide](docs/TESTING.md) - Unit and integration tests
- [Troubleshooting](docs/TROUBLESHOOTING.md) - Common issues and solutions
- [Calibration](docs/CALIBRACION.md) - Device settings and calibration
- [Coding Rules](docs/RULES.md) - Code style and conventions
- [Todo List](docs/TODO.md) - Planned features and improvements
- [Changelog](docs/CHANGELOG.md) - Version history

## License

MIT License - See LICENSE file for details.

## Author

Professional embedded C++ project for Raspberry Pi.
