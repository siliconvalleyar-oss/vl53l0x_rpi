# TODO List

## High Priority

- [ ] Implement complete VL53L0X register map (VL53L0X_regs.hpp)
- [ ] Add error handling with std::optional for medir()
- [ ] Implement calibration routine with reference distance
- [ ] Add unit tests for I2C register read/write
- [ ] Verify compilation on Raspberry Pi 3, 4, and 5

## Medium Priority

- [ ] Implement OLED display output (SSD1306)
- [ ] Add logging system with severity levels
- [ ] Create example applications:
  - [ ] Distance threshold alarm
  - [ ] Data logging to file
  - [ ] Web server with distance readings
- [ ] Add configuration file support (JSON/YAML)
- [ ] Implement multi-sensor support (different I2C addresses)

## Low Priority

- [ ] Add Python bindings via pybind11
- [ ] Create Docker image for testing
- [ ] Add calibration wizard script
- [ ] Implement distance averaging/filtering
- [ ] Add ROS (Robot Operating System) node
- [ ] Create web interface for configuration

## Documentation

- [ ] Add video tutorial for hardware setup
- [ ] Create troubleshooting flowcharts
- [ ] Document performance benchmarks
- [ ] Add examples for different use cases

## Testing

- [ ] Set up CI/CD with GitHub Actions
- [ ] Add hardware-in-the-loop tests
- [ ] Create mock I2C for unit testing
- [ ] Add coverage reports

## Known Issues

- [ ] I2C speed limited to 400kHz (bcm2835 limitation)
- [ ] Requires sudo for execution (bcm2835 requirement)
- [ ] No timeout handling for sensor response
