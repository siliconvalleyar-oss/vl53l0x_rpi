#include "gpio_wrapper.hpp"
#include "lib/bcm2835_init.hpp"

namespace UTILS {

GpioPin::GpioPin(uint8_t pin, Direction direction)
    : pin_(pin), initialized_(false) {
    Bcm2835Init::initialize();
    bcm2835_gpio_fsel(pin_, BCM2835_GPIO_FSEL_OUTP);
    set_direction(direction);
    initialized_ = true;
}

GpioPin::~GpioPin() {
    initialized_ = false;
}

void GpioPin::set_state(State state) {
    if (!initialized_) return;
    bcm2835_gpio_write(pin_, static_cast<uint8_t>(state));
}

GpioPin::State GpioPin::get_state() const {
    if (!initialized_) return State::PIN_LOW;
    uint8_t level = bcm2835_gpio_lev(pin_);
    return level ? State::PIN_HIGH : State::PIN_LOW;
}

void GpioPin::set_direction(Direction direction) {
    if (!initialized_) return;
    uint8_t mode = (direction == Direction::OUTPUT)
        ? BCM2835_GPIO_FSEL_OUTP
        : BCM2835_GPIO_FSEL_INPT;
    bcm2835_gpio_fsel(pin_, mode);
}

}
