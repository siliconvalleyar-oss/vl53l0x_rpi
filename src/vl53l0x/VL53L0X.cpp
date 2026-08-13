#include "vl53l0x/VL53L0X.hpp"
#include "vl53l0x/VL53L0X_impl.hpp"
#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstring>

namespace VL53L0X {

std::atomic<bool> g_running(true);

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_running = false;
    }
}

Vl53l0x_t::Vl53l0x_t()
    : pimpl(std::make_unique<Impl>()) {
}

Vl53l0x_t::~Vl53l0x_t() = default;

bool Vl53l0x_t::init() {
    if (!pimpl) return false;
    return pimpl->initialize();
}

uint16_t Vl53l0x_t::medir() {
    if (!pimpl) return 0;
    return pimpl->read_distance();
}

void Vl53l0x_t::calibrar(int16_t offset) {
    if (!pimpl) return;
    pimpl->apply_offset(offset);
}

void Vl53l0x_t::sleep() {
    if (!pimpl) return;
    pimpl->sleep();
}

void Vl53l0x_t::wake() {
    if (!pimpl) return;
    pimpl->wake();
}

void Vl53l0x_t::set_measurement_mode(MeasurementMode mode) {
    if (!pimpl) return;
    pimpl->set_mode(mode);
}

uint8_t Vl53l0x_t::get_address() const {
    return VL53L0X_DEFAULT_ADDRESS;
}

int16_t Vl53l0x_t::get_offset() const {
    return 0;
}

bool Vl53l0x_t::is_initialized() const {
    return true;
}

const char* get_error_string(Vl53l0xError error) {
    switch (error) {
        case Vl53l0xError::NONE:
            return "No error";
        case Vl53l0xError::I2C_ERROR:
            return "I2C communication error";
        case Vl53l0xError::INVALID_DEVICE:
            return "Invalid device ID";
        case Vl53l0xError::TIMEOUT:
            return "Operation timeout";
        case Vl53l0xError::CALIBRATION_ERROR:
            return "Calibration error";
        case Vl53l0xError::NOT_INITIALIZED:
            return "Sensor not initialized";
        default:
            return "Unknown error";
    }
}

}
