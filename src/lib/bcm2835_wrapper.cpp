#include "lib/bcm2835_wrapper.hpp"
#include "lib/bcm2835_init.hpp"

namespace UTILS {

I2cDevice::I2cDevice(uint8_t address)
    : address_(address), initialized_(false), last_error_(false) {
    Bcm2835Init::initialize();
    bcm2835_i2c_begin();
    bcm2835_i2c_setSlaveAddress(address_);
    bcm2835_i2c_set_baudrate(100000);
    initialized_ = true;
}

I2cDevice::~I2cDevice() {
    if (initialized_) {
        bcm2835_i2c_end();
    }
}

bool I2cDevice::write_register(uint8_t reg, uint8_t value) {
    if (!initialized_) {
        last_error_ = true;
        return false;
    }
    char buffer[2] = {static_cast<char>(reg), static_cast<char>(value)};
    bool ok = bcm2835_i2c_write(buffer, 2) == BCM2835_I2C_REASON_OK;
    last_error_ = !ok;
    return ok;
}

uint8_t I2cDevice::read_register(uint8_t reg) {
    last_error_ = false;
    if (!initialized_) {
        last_error_ = true;
        return 0xFF;
    }
    bcm2835_i2c_setSlaveAddress(address_);
    if (bcm2835_i2c_write(reinterpret_cast<char*>(&reg), 1) != BCM2835_I2C_REASON_OK) {
        last_error_ = true;
        return 0xFF;
    }
    char buf[1];
    if (bcm2835_i2c_read(buf, 1) != BCM2835_I2C_REASON_OK) {
        last_error_ = true;
        return 0xFF;
    }
    return static_cast<uint8_t>(buf[0]);
}

bool I2cDevice::read_register(uint8_t reg, uint8_t* buffer, uint16_t length) {
    last_error_ = false;
    if (!initialized_ || !buffer || length == 0) {
        last_error_ = true;
        return false;
    }
    bcm2835_i2c_setSlaveAddress(address_);
    if (bcm2835_i2c_write(reinterpret_cast<char*>(&reg), 1) != BCM2835_I2C_REASON_OK) {
        last_error_ = true;
        return false;
    }
    bool ok = bcm2835_i2c_read(reinterpret_cast<char*>(buffer), length) == BCM2835_I2C_REASON_OK;
    last_error_ = !ok;
    return ok;
}

}
