#include "vl53l0x/VL53L0X_impl.hpp"
#include "vl53l0x/VL53L0X_regs.hpp"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>

namespace VL53L0X {

Vl53l0x_t::Impl::Impl()
    : address_(VL53L0X_DEFAULT_ADDRESS)
    , offset_(0)
    , is_initialized_(false)
    , mode_(MeasurementMode::SINGLE_SHOT)
    , last_distance_(0) {
}

Vl53l0x_t::Impl::~Impl() = default;

void Vl53l0x_t::Impl::set_xshut_pin(uint8_t pin) {
    if (pin == 0) return;
    try {
        xshut_ = std::make_unique<UTILS::GpioPin>(pin, UTILS::GpioPin::Direction::OUTPUT);
        xshut_->set_state(UTILS::GpioPin::State::PIN_LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        xshut_->set_state(UTILS::GpioPin::State::PIN_HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } catch (const std::exception& e) {
        xshut_.reset();
    }
}

void Vl53l0x_t::Impl::set_gpio1_pin(uint8_t pin) {
    if (pin == 0) return;
    try {
        gpio1_ = std::make_unique<UTILS::GpioPin>(pin, UTILS::GpioPin::Direction::INPUT);
    } catch (const std::exception& e) {
        gpio1_.reset();
    }
}

bool Vl53l0x_t::Impl::hardware_reset() {
    if (!xshut_) return false;
    xshut_->set_state(UTILS::GpioPin::State::PIN_LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    xshut_->set_state(UTILS::GpioPin::State::PIN_HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return true;
}

bool Vl53l0x_t::Impl::wait_for_device(uint8_t /*address*/, int timeout_ms) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count() < timeout_ms) {
        uint8_t id = read_reg(VL53L0X_REG_IDENTIFICATION_MODEL_ID);
        if (id == VL53L0X_DEVICE_ID) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}

bool Vl53l0x_t::Impl::initialize() {
    if (xshut_) {
        xshut_->set_state(UTILS::GpioPin::State::PIN_LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        xshut_->set_state(UTILS::GpioPin::State::PIN_HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    try {
        i2c_ = std::make_unique<UTILS::I2cDevice>(address_);
    } catch (const std::exception& e) {
        return false;
    }

    uint8_t device_id = read_reg(VL53L0X_REG_IDENTIFICATION_MODEL_ID);
    if (device_id != VL53L0X_DEVICE_ID) {
        return false;
    }

    write_reg(0x88, 0x00);
    write_reg(0x80, 0x01);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);

    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0xFF, 0x06);
    uint8_t addr = read_reg(0x83);
    write_reg(0x83, addr | 0x04);
    write_reg(0xFF, 0x07);
    write_reg(0x81, 0x01);
    write_reg(0x80, 0x01);
    write_reg(0x94, 0x6B);
    write_reg(0x83, 0x00);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    uint8_t ssc_finish = 0;
    for (int i = 0; i < 100; ++i) {
        ssc_finish = read_reg(0x83);
        if (ssc_finish & 0x01) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    write_reg(0x83, 0x00);
    write_reg(0xFF, 0x06);
    write_reg(0x83, addr & ~0x04);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x01);

    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x09, 0x00);
    write_reg(0x10, 0x00);
    write_reg(0x11, 0x00);
    write_reg(0x24, 0x00);
    write_reg(0x25, 0x00);
    write_reg(0x75, 0x00);
    write_reg(0xFF, 0x01);
    write_reg(0x4E, 0x2C);
    write_reg(0x48, 0x00);
    write_reg(0x30, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x30, 0x01);
    write_reg(0x54, 0xFF);
    write_reg(0x32, 0x00);
    write_reg(0x89, 0x00);
    write_reg(0x57, 0x00);
    write_reg(0xFF, 0x01);
    write_reg(0x05, 0x01);
    write_reg(0x06, 0x01);
    write_reg(0x07, 0x00);
    write_reg(0x0A, 0x00);
    write_reg(0x0B, 0x00);
    write_reg(0x0C, 0x00);
    write_reg(0x0D, 0x00);
    write_reg(0x0F, 0x00);
    write_reg(0x10, 0x00);
    write_reg(0x11, 0x00);
    write_reg(0x14, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x0A, 0x04);
    write_reg(0x84, 0x00);
    write_reg(0x0B, 0x05);
    write_reg(0xFF, 0x01);
    write_reg(0x01, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x1E, 0x00);

    is_initialized_ = true;
    return true;
}

uint16_t Vl53l0x_t::Impl::read_distance() {
    if (!is_initialized_) return 0;

    write_reg(VL53L0X_REG_SYSRANGE_START, 0x01);

    for (int i = 0; i < 100; ++i) {
        uint8_t status = read_reg(VL53L0X_REG_RESULT_INTERRUPT_STATUS);
        if (status & 0x01) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);

    uint8_t high = read_reg(VL53L0X_REG_RESULT_RANGE_STATUS + 10);
    uint8_t low = read_reg(VL53L0X_REG_RESULT_RANGE_STATUS + 11);
    uint16_t distance = (static_cast<uint16_t>(high) << 8) | low;
    std::cerr << "Debug: high=" << (int)high << " low=" << (int)low << " raw=" << distance << std::endl;
    distance = calculate_distance(distance);

    last_distance_ = distance;
    return distance;
}

void Vl53l0x_t::Impl::apply_offset(int16_t offset) {
    offset_ = offset;
    if (offset != 0 && offset > -100 && offset < 100) {
        write_reg(VL53L0X_REG_RESULT_OFFSET, static_cast<uint8_t>(offset));
    }
}

void Vl53l0x_t::Impl::set_mode(MeasurementMode mode) {
    mode_ = mode;
    switch (mode) {
        case MeasurementMode::SINGLE_SHOT:
            write_reg(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT);
            break;
        case MeasurementMode::CONTINUOUS:
            write_reg(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
            break;
        case MeasurementMode::TIMED:
            write_reg(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_TIMED);
            break;
    }
}

void Vl53l0x_t::Impl::sleep() {
    if (!is_initialized_) return;
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0x91, 0x00);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);
}

void Vl53l0x_t::Impl::wake() {
    if (!is_initialized_) return;
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);
}

bool Vl53l0x_t::Impl::write_reg(uint8_t reg, uint8_t value) {
    if (!i2c_) return false;
    return i2c_->write_register(reg, value);
}

uint8_t Vl53l0x_t::Impl::read_reg(uint8_t reg) {
    if (!i2c_) return 0;
    return i2c_->read_register(reg);
}

bool Vl53l0x_t::Impl::read_regs(uint8_t reg, uint8_t* buffer, uint16_t length) {
    if (!i2c_ || !buffer) return false;
    return i2c_->read_register(reg, buffer, length);
}

uint16_t Vl53l0x_t::Impl::calculate_distance(uint16_t raw) {
    if (raw == 0 || raw == 0xFFFF) return 0;
    int32_t distance = static_cast<int32_t>(raw) + offset_;
    if (distance < static_cast<int32_t>(VL53L0X_MIN_DISTANCE)) return VL53L0X_MIN_DISTANCE;
    if (distance > static_cast<int32_t>(VL53L0X_MAX_DISTANCE)) return VL53L0X_MAX_DISTANCE;
    return static_cast<uint16_t>(distance);
}

}
