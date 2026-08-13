#pragma once

#include <memory>
#include "VL53L0X.hpp"
#include "lib/bcm2835_wrapper.hpp"
#include "lib/gpio_wrapper.hpp"

namespace VL53L0X {

class Vl53l0x_t::Impl {
public:
    Impl();
    ~Impl();

    bool initialize();
    uint16_t read_distance();
    void apply_offset(int16_t offset);
    void set_mode(MeasurementMode mode);
    void sleep();
    void wake();
    bool hardware_reset();

    void set_xshut_pin(uint8_t pin);
    void set_gpio1_pin(uint8_t pin);

    int16_t get_offset() const { return offset_; }
    bool is_initialized() const { return is_initialized_; }
    uint16_t get_last_distance() const { return last_distance_; }

private:
    bool write_reg(uint8_t reg, uint8_t value);
    uint8_t read_reg(uint8_t reg);
    bool read_regs(uint8_t reg, uint8_t* buffer, uint16_t length);
    uint16_t calculate_distance(uint16_t raw);
    bool wait_for_device(uint8_t address, int timeout_ms);

    std::unique_ptr<UTILS::I2cDevice> i2c_;
    std::unique_ptr<UTILS::GpioPin> xshut_;
    std::unique_ptr<UTILS::GpioPin> gpio1_;
    uint8_t address_;
    int16_t offset_;
    bool is_initialized_;
    MeasurementMode mode_;
    uint16_t last_distance_;
};

}
