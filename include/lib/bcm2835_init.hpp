#pragma once

#include <bcm2835.h>
#include <cstdint>
#include <stdexcept>
#include <atomic>

namespace UTILS {

class Bcm2835Init {
public:
    static void initialize() {
        static std::atomic<bool> initialized{false};
        if (initialized.load()) return;
        if (!bcm2835_init()) {
            throw std::runtime_error("Failed to initialize bcm2835");
        }
        initialized.store(true);
    }
};

}
