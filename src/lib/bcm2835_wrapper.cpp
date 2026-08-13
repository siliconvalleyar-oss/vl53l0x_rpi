#include "lib/bcm2835_wrapper.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

namespace UTILS {

namespace {
constexpr const char* kI2cBusPath = "/dev/i2c-1";
}

I2cDevice::I2cDevice(uint8_t address)
    : address_(address), fd_(-1), initialized_(false), last_error_(false) {
    fd_ = open(kI2cBusPath, O_RDWR);
    if (fd_ < 0) {
        last_error_ = true;
        return;
    }
    if (ioctl(fd_, I2C_SLAVE, address_) < 0) {
        last_error_ = true;
        close(fd_);
        fd_ = -1;
        return;
    }
    initialized_ = true;
}

I2cDevice::~I2cDevice() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

bool I2cDevice::write_register(uint8_t reg, uint8_t value) {
    last_error_ = false;
    if (!initialized_) {
        last_error_ = true;
        return false;
    }
    uint8_t buffer[2] = {reg, value};
    struct i2c_msg msg = {};
    msg.addr = address_;
    msg.flags = 0;
    msg.len = 2;
    msg.buf = buffer;
    struct i2c_rdwr_ioctl_data data = {};
    data.msgs = &msg;
    data.nmsgs = 1;
    bool ok = ioctl(fd_, I2C_RDWR, &data) >= 0;
    last_error_ = !ok;
    return ok;
}

uint8_t I2cDevice::read_register(uint8_t reg) {
    uint8_t value = 0xFF;
    read_register(reg, &value, 1);
    return value;
}

bool I2cDevice::read_register(uint8_t reg, uint8_t* buffer, uint16_t length) {
    last_error_ = false;
    if (!initialized_ || !buffer || length == 0) {
        last_error_ = true;
        return false;
    }
    struct i2c_msg msgs[2] = {};
    msgs[0].addr = address_;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;
    msgs[1].addr = address_;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = length;
    msgs[1].buf = buffer;
    struct i2c_rdwr_ioctl_data data = {};
    data.msgs = msgs;
    data.nmsgs = 2;
    bool ok = ioctl(fd_, I2C_RDWR, &data) >= 0;
    last_error_ = !ok;
    return ok;
}

}
