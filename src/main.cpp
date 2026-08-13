#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include "vl53l0x/VL53L0X.hpp"

int main() {
    std::signal(SIGINT, VL53L0X::signal_handler);
    std::signal(SIGTERM, VL53L0X::signal_handler);

    std::cout << "Iniciando sensor VL53L0X..." << std::endl;

    auto laser = std::make_unique<VL53L0X::Vl53l0x_t>();

    laser->set_xshut_pin(17);
    laser->set_gpio1_pin(4);

    if (!laser->init()) {
        std::cerr << "Error: No se pudo inicializar el sensor VL53L0X" << std::endl;
        return 1;
    }

    std::cout << "Sensor inicializado correctamente." << std::endl;
    std::cout << "Presione Ctrl+C para salir" << std::endl;
    std::cout << std::endl;

    laser->set_measurement_mode(VL53L0X::MeasurementMode::CONTINUOUS);
    laser->calibrar(0);

    while (VL53L0X::g_running) {
        uint16_t distance = laser->medir();

        if (distance > 0 && distance < 2000) {
            std::cout << "Distancia: " << distance << " mm" << std::endl;
        } else if (distance == 0) {
            std::cout << "Distancia: fuera de rango" << std::endl;
        } else {
            std::cout << "Distancia: " << distance << " mm (error)" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << std::endl;
    std::cout << "Saliendo del programa... Liberando recursos." << std::endl;
    laser->sleep();
    laser.reset();

    std::cout << "Memoria liberada correctamente." << std::endl;
    return 0;
}
