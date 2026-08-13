# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-01-15

### Added
- Initial project structure
- VL53L0X sensor wrapper class
- I2C communication via bcm2835
- Smart pointer memory management
- Makefile with cross-architecture support
- Installation and build scripts
- Complete documentation suite

### Features
- Basic distance measurement
- Calibration offset support
- Sleep/wake low-power modes
- Continuous measurement mode
- Signal handling for clean shutdown

### Documentation
- README.md with quick start guide
- Architecture documentation with diagrams
- Complete API reference
- Coding rules and conventions
- Setup guide for Raspberry Pi
- Troubleshooting guide
- Testing documentation

### Scripts
- install_deps.sh - Dependency installation
- build.sh - Project build script
- run.sh - Execution with sudo
- clean.sh - Cleanup script

## [Unreleased]

### Planned
- OLED display support (SSD1306)
- Calibration wizard
- Multi-sensor support
- Web interface
- ROS integration
- Python bindings
