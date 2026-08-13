#include "include/lib/gpio_wrapper.hpp"

namespace UTILS {

GpioPin::GpioPin(uint8_t pin, Direction direction)
    : pin_(pin), initialized_(false) {
    if (!bcm2835_init()) {
        throw std::runtime_error("Failed to initialize bcm2835 for GPIO");
    }

    bcm2835_gpio_fsel(pin_, BCM2835_GPIO_FSEL_OUTPT);
    set_direction(direction);
    initialized_ = true;
}

GpioPin::~GpioPin() {
    if (initialized_) {
        bcm2835_close();
    }
}

void GpioPin::set_state(State state) {
    if (!initialized_) return;
    bcm2835_gpio_write(pin_, static_cast<uint8_t>(state));
}

GpioPin::State GpioPin::get_state() const {
    if (!initialized_) return State::LOW;
    uint8_t level = bcm2835_gpio_lev(pin_);
    return level ? State::HIGH : State::LOW;
}

void GpioPin::set_direction(Direction direction) {
    if (!initialized_) return;
    uint8_t mode = (direction == Direction::OUTPUT)
        ? BCM2835_GPIO_FSEL_OUTPT
        : BCM2835_GPIO_FSEL_INPT;
    bcm2835_gpio_fsel(pin_, mode);
}

}
