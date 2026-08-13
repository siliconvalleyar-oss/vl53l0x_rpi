#include "lib/bcm2835_wrapper.hpp"

namespace UTILS {

I2cDevice::I2cDevice(uint8_t address)
    : address_(address), initialized_(false) {
    if (!bcm2835_init()) {
        throw std::runtime_error("Failed to initialize bcm2835");
    }
    if (!bcm2835_i2c_begin()) {
        bcm2835_close();
        throw std::runtime_error("Failed to initialize I2C");
    }
    bcm2835_i2c_setSlaveAddress(address_);
    initialized_ = true;
}

I2cDevice::~I2cDevice() {
    if (initialized_) {
        bcm2835_i2c_end();
        bcm2835_close();
    }
}

bool I2cDevice::write_register(uint8_t reg, uint8_t value) {
    if (!initialized_) return false;
    uint8_t buffer[2] = {reg, value};
    return bcm2835_i2c_write((char*)buffer, 2) == BCM2835_I2C_REASON_OK;
}

bool I2cDevice::read_register(uint8_t reg, uint8_t* buffer, uint16_t length) {
    if (!initialized_ || !buffer) return false;
    bcm2835_i2c_setSlaveAddress(address_);
    return bcm2835_i2c_read_register_rs(reinterpret_cast<char*>(const_cast<uint8_t*>(&reg)), reinterpret_cast<char*>(buffer), static_cast<uint32_t>(length)) == BCM2835_I2C_REASON_OK;
}

uint8_t I2cDevice::read_register(uint8_t reg) {
    if (!initialized_) return 0;
    char buffer[1];
    bcm2835_i2c_setSlaveAddress(address_);
    if (bcm2835_i2c_read_register_rs(reinterpret_cast<char*>(const_cast<uint8_t*>(&reg)), buffer, 1) != BCM2835_I2C_REASON_OK) {
        return 0;
    }
    return static_cast<uint8_t>(buffer[0]);
}

}
