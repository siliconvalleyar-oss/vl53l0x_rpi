# Troubleshooting Guide

## Common Issues and Solutions

### Issue: "Permission denied" when running

**Symptoms:**
```
bcm2835_init: Unable to open /dev/mem : Permission denied
```

**Solution:**
Run with sudo:
```bash
sudo ./bin/laser_measure
```

Or configure udev rules (see SETUP.md).

### Issue: "No device found" at address 0x29

**Symptoms:**
```
Inicializando sensor VL53L0X...
Error: Sensor no encontrado en direccion 0x29
```

**Solutions:**
1. Verify I2C is enabled: `sudo raspi-config` → Interface Options → I2C
2. Check wiring connections
3. Verify I2C address: `sudo i2cdetect -y 1`
4. Ensure sensor is powered (3.3V)

### Issue: Compilation fails with bcm2835 errors

**Symptoms:**
```
fatal error: bcm2835.h: No such file or directory
```

**Solution:**
Install bcm2835 library:
```bash
sudo ./scripts/install_deps.sh
```

Or manually:
```bash
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
tar -zxvf bcm2835-1.71.tar.gz
cd bcm2835-1.71
./configure && make && sudo make install
```

### Issue: Distance readings are 0 or 65535

**Symptoms:**
```
Distancia: 0 mm
Distancia: 65535 mm
```

**Solutions:**
1. Sensor not initialized - call `init()` before `medir()`
2. I2C communication error - check wiring
3. Sensor in error state - call `init()` again
4. Object too close (< 30mm) or too far (> 2000mm)

### Issue: Measurements are inconsistent

**Symptoms:**
```
Distancia: 245 mm
Distancia: 180 mm  // Spike
Distancia: 250 mm
```

**Solutions:**
1. Perform calibration: `sensor->calibrar(offset)`
2. Enable averaging in code
3. Check for reflective surface issues
4. Ensure stable power supply

### Issue: "Cannot open I2C device"

**Symptoms:**
```
Error: No puedo abrir el dispositivo I2C
```

**Solutions:**
1. Ensure I2C is enabled
2. Run with sudo
3. Check `/dev/i2c-1` exists

### Issue: Build fails with "architecture not supported"

**Symptoms:**
```
Error: unsupported CPU architecture
```

**Solutions:**
The Makefile should auto-detect architecture. If not:
```bash
# For 32-bit Raspberry Pi
make ARCH=armv7l

# For 64-bit Raspberry Pi
make ARCH=aarch64
```

## Debug Mode

Enable debug output:

```cpp
#define DEBUG_MODE
```

Or compile with:
```bash
make CXXFLAGS="-g -DDEBUG"
```

## Performance Issues

### Slow measurements
- Reduce timing budget in configuration
- Use continuous mode instead of single-shot
- Check I2C bus speed (should be 400kHz)

### High CPU usage
- Increase sleep interval between measurements
- Use continuous mode with inter-measurement period

## Getting Help

If issues persist:
1. Check wiring and power supply
2. Verify I2C address with `i2cdetect`
3. Enable debug logging
4. Check Raspberry Pi model and OS version
5. Review bcm2835 library documentation
