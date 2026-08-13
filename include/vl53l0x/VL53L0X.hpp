#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <atomic>

namespace VL53L0X {

extern std::atomic<bool> g_running;
void signal_handler(int signal);

enum class MeasurementMode {
    SINGLE_SHOT = 0,
    CONTINUOUS = 1,
    TIMED = 2
};

enum class Vl53l0xError {
    NONE = 0,
    I2C_ERROR = 1,
    INVALID_DEVICE = 2,
    TIMEOUT = 3,
    CALIBRATION_ERROR = 4,
    NOT_INITIALIZED = 5
};

struct Vl53l0xConfig {
    uint16_t timing_budget_ms;
    uint16_t inter_measurement_period_ms;
    uint8_t vhv_config;
    bool continuous_mode;
};

constexpr uint8_t VL53L0X_DEFAULT_ADDRESS = 0x29;
constexpr uint16_t VL53L0X_MAX_DISTANCE = 2000;
constexpr uint16_t VL53L0X_MIN_DISTANCE = 30;

class Vl53l0x_t {
public:
    Vl53l0x_t();
    ~Vl53l0x_t();

    Vl53l0x_t(const Vl53l0x_t&) = delete;
    Vl53l0x_t& operator=(const Vl53l0x_t&) = delete;

    bool init();
    uint16_t medir();
    void calibrar(int16_t offset);
    void sleep();
    void wake();
    void set_measurement_mode(MeasurementMode mode);

    uint8_t get_address() const;
    int16_t get_offset() const;
    bool is_initialized() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

const char* get_error_string(Vl53l0xError error);

}
