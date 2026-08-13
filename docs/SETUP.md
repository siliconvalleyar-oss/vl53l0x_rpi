# Raspberry Pi Setup Guide

## Initial Configuration

### 1. Update System

```bash
sudo apt update && sudo apt upgrade -y
sudo reboot
```

### 2. Enable I2C Interface

```bash
sudo raspi-config
```

Navigate to:
- Interface Options
- I2C
- Enable I2C interface
- Reboot when prompted

### 3. Install Build Dependencies

```bash
sudo apt install -y build-essential git cmake
```

### 4. Install bcm2835 Library

```bash
# Download latest version
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
tar -zxvf bcm2835-1.71.tar.gz
cd bcm2835-1.71
./configure
make
sudo make check
sudo make install
cd ..
rm -rf bcm2835-1.71 bcm2835-1.71.tar.gz
```

### 5. Verify I2C Connection

```bash
# Install i2c-tools
sudo apt install -y i2c-tools

# Scan I2C bus for VL53L0X (address 0x29)
sudo i2cdetect -y 1

# Expected output:
#      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
# 00:          -- -- -- -- -- -- -- -- -- -- -- -- --
# 10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# ...
# 29: 29 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
```

### 6. Configure Permissions (Optional)

For non-sudo execution (not recommended):

```bash
# Add user to i2c group
sudo usermod -aG i2c $USER

# Add udev rule for bcm2835
echo 'SUBSYSTEM=="gpio", KERNEL=="gpiochip*", ACTION=="add", RUN+="/bin/chown root:i2c /dev/gpiochip*", MODE="0660"' | sudo tee /etc/udev/rules.d/99-gpio.rules

# Reboot
sudo reboot
```

## Hardware Setup

### VL53L0X to Raspberry Pi Connections

```
VL53L0X Module    Raspberry Pi GPIO    Physical Pin
──────────────────────────────────────────────────
VIN              3.3V                 Pin 1 (Red wire)
GND              GND                  Pin 6 (Black wire)
SDA              GPIO2 (SDA1)         Pin 3 (Green wire)
SCL              GPIO3 (SCL1)         Pin 5 (Yellow wire)
```

### Verification

```bash
# Check I2C is working
ls /dev/i2c-1

# Test with i2cdetect
sudo i2cdetect -y 1
```

## Project Setup

```bash
# Clone repository
git clone <repository-url> vl53l0x_project
cd vl53l0x_project

# Install dependencies
sudo ./scripts/install_deps.sh

# Build project
./scripts/build.sh

# Run sensor
sudo ./scripts/run.sh
```

## Troubleshooting Setup

- If I2C not detected: Check wiring and enable I2C in raspi-config
- If permission denied: Run with sudo or configure udev rules
- If sensor not found: Verify address 0x29 with i2cdetect
