#pragma once

#include <bcm2835.h>
#include <cstdint>
#include <stdexcept>

namespace UTILS {

class GpioPin {
public:
    enum class Direction {
        INPUT = 0,
        OUTPUT = 1
    };

    enum class State {
        PIN_LOW = 0,
        PIN_HIGH = 1
    };

    GpioPin(uint8_t pin, Direction direction);
    ~GpioPin();

    void set_state(State state);
    State get_state() const;
    void set_direction(Direction direction);

private:
    uint8_t pin_;
    bool initialized_;
};

}
