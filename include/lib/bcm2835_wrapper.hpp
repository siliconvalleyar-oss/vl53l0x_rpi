#pragma once

#include <bcm2835.h>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace UTILS {

class I2cDevice {
public:
    I2cDevice(uint8_t address);
    ~I2cDevice();

    bool write_register(uint8_t reg, uint8_t value);
    bool read_register(uint8_t reg, uint8_t* buffer, uint16_t length);
    uint8_t read_register(uint8_t reg);

private:
    uint8_t address_;
    bool initialized_;
};

}
