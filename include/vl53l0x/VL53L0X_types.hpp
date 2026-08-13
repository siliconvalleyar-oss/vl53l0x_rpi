#pragma once

#include <cstdint>

namespace VL53L0X {

using Vl53l0xError = Vl53l0xError;
using Vl53l0xConfig = Vl53l0xConfig;
using MeasurementMode = MeasurementMode;

constexpr uint16_t MAX_DISTANCE_MM = 2000;
constexpr uint16_t MIN_DISTANCE_MM = 30;
constexpr uint8_t DEFAULT_ADDRESS = 0x29;
constexpr uint16_t TIMING_BUDGET_DEFAULT_MS = 20;

}
