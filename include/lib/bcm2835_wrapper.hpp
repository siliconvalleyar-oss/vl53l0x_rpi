#pragma once

#include <cstdint>

namespace UTILS {

class I2cDevice {
public:
    I2cDevice(uint8_t address);
    ~I2cDevice();

    bool write_register(uint8_t reg, uint8_t value);
    uint8_t read_register(uint8_t reg);
    bool read_register(uint8_t reg, uint8_t* buffer, uint16_t length);
    bool last_error() const { return last_error_; }

private:
    uint8_t address_;
    int fd_;
    bool initialized_;
    bool last_error_;
};

}
