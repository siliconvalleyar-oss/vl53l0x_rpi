#include "vl53l0x/VL53L0X_impl.hpp"
#include "vl53l0x/VL53L0X_regs.hpp"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>

namespace VL53L0X {

namespace {
// Calibracion empirica del modulo (SSC no completo):
//   medido = 1.4644 x real + 23  =>  real = (medido - 23) x 100 / 146
// Puntos de referencia: 30 mm -> 67 mm, 297 mm (A4) -> 458 mm
constexpr int32_t kCalibRawToMmOffset = 23;
constexpr int32_t kCalibRawToMmScale = 146; // (x100)
}

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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        xshut_->set_state(UTILS::GpioPin::State::PIN_HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        xshut_->set_state(UTILS::GpioPin::State::PIN_HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    try {
        i2c_ = std::make_unique<UTILS::I2cDevice>(address_);
    } catch (const std::exception& e) {
        return false;
    }

    // ---- VL53L0X_DataInit: boot/wake del firmware (sale del estado stop) ----
    write_reg(0x88, 0x00);
    write_reg(0x80, 0x01);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    read_reg(0x91); // boot status
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    if (!wait_for_device(address_, 200)) {
        return false;
    }

    uint8_t device_id = read_reg(VL53L0X_REG_IDENTIFICATION_MODEL_ID);
    if (device_id != VL53L0X_DEVICE_ID) {
        return false;
    }

    // ---- VL53L0X_StaticInit: boot/VCSEL sequence ----
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

    bool ssc_ok = false;
    for (int i = 0; i < 100; ++i) {
        uint8_t ssc_finish = read_reg(0x83);
        if (ssc_finish & 0x01) {
            ssc_ok = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!ssc_ok) {
        std::cerr << "Aviso: calibracion SSC/SPAD no completo; "
                  << "continuando sin calibracion de referencia." << std::endl;
    }

    write_reg(0x83, 0x00);
    write_reg(0xFF, 0x06);
    write_reg(0x83, addr & ~0x04);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    // ---- GPIO interrupt config: interrupt on new sample ready ----
    write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);

    // ---- Desactivar TCC y MSRC por defecto ----
    write_reg(VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);

    is_initialized_ = true;
    return true;
}

uint16_t Vl53l0x_t::Impl::read_distance() {
    if (!is_initialized_) return 0xFFFF;

    if (mode_ == MeasurementMode::SINGLE_SHOT) {
        if (!write_reg(VL53L0X_REG_SYSRANGE_START, 0x01)) {
            return 0xFFFF;
        }
    }

    bool ready = false;
    for (int i = 0; i < 100; ++i) {
        uint8_t status = read_reg(VL53L0X_REG_RESULT_INTERRUPT_STATUS);
        if (i2c_ && i2c_->last_error()) {
            return 0xFFFF;
        }
        if (status & 0x07) {
            ready = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (!ready) {
        if (mode_ == MeasurementMode::SINGLE_SHOT) {
            write_reg(VL53L0X_REG_SYSRANGE_START, 0x00);
        }
        return 0;
    }

    uint8_t high = read_reg(VL53L0X_REG_RESULT_RANGE_STATUS + 10);
    if (i2c_ && i2c_->last_error()) {
        return 0xFFFF;
    }
    uint8_t low = read_reg(VL53L0X_REG_RESULT_RANGE_STATUS + 11);
    if (i2c_ && i2c_->last_error()) {
        return 0xFFFF;
    }

    write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);

    uint16_t distance = (static_cast<uint16_t>(high) << 8) | low;
    distance = calculate_distance(distance);

    last_distance_ = distance;
    return distance;
}

void Vl53l0x_t::Impl::apply_offset(int16_t offset) {
    offset_ = offset;
    if (offset != 0) {
        write_reg(VL53L0X_REG_ALGO_PART_TO_PART_OFFSET, static_cast<uint8_t>(offset));
    }
}

void Vl53l0x_t::Impl::set_mode(MeasurementMode mode) {
    mode_ = mode;
    if (!is_initialized_) return;
    switch (mode) {
        case MeasurementMode::SINGLE_SHOT:
            write_reg(VL53L0X_REG_SYSRANGE_START, 0x00);
            break;
        case MeasurementMode::CONTINUOUS:
            write_reg(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK | 0x01);
            break;
        case MeasurementMode::TIMED:
            write_reg(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_TIMED | 0x01);
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
    if (raw == 0xFFFF || raw == 0x1FFF) return 0;
    int32_t corrected = (static_cast<int32_t>(raw) - kCalibRawToMmOffset) * 100 / kCalibRawToMmScale;
    if (corrected < 0) corrected = 0;
    return static_cast<uint16_t>(corrected);
}

}
