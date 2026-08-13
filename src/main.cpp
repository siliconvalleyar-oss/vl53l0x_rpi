#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include "vl53l0x/VL53L0X.hpp"

#ifndef APP_VERSION
#define APP_VERSION "desconocida"
#endif

int main() {
    std::signal(SIGINT, VL53L0X::signal_handler);
    std::signal(SIGTERM, VL53L0X::signal_handler);

    std::cout << "=============================================" << std::endl;
    std::cout << "  Laser Ranger (VL53L0X)  v" << APP_VERSION << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "  Medidor de distancia por Time-of-Flight" << std::endl;
    std::cout << "  Modo de prueba: 16 segundos" << std::endl;
    std::cout << "  Presione Ctrl+C para salir antes" << std::endl;
    std::cout << "  Compilado: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::endl;

    std::cout << "Iniciando sensor VL53L0X..." << std::endl;

    auto laser = std::make_unique<VL53L0X::Vl53l0x_t>();

    laser->set_xshut_pin(17);
    laser->set_gpio1_pin(27);

    if (!laser->init()) {
        std::cerr << "Error: No se pudo inicializar el sensor VL53L0X" << std::endl;
        return 1;
    }

    std::cout << "Sensor inicializado correctamente." << std::endl;
    std::cout << "Presione Ctrl+C para salir" << std::endl;
    std::cout << std::endl;

    laser->set_measurement_mode(VL53L0X::MeasurementMode::SINGLE_SHOT);
    laser->calibrar(0);

    // Filtro de mediana movil para estabilizar el ruido del modulo
    // (VCSEL degradado: lecturas individuales varian ±6 mm). Ventana de 9
    // para mayor estabilidad con objeto fijo; responder mas lento a cambios.
    std::vector<uint16_t> window;
    window.reserve(9);
    constexpr size_t kWindowSize = 9;

    auto start = std::chrono::steady_clock::now();

    while (VL53L0X::g_running) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start).count();
        if (elapsed >= 16) {
            std::cout << "Tiempo maximo de prueba alcanzado (16s)." << std::endl;
            break;
        }

        uint16_t distance = laser->medir();

        if (distance == 0xFFFF) {
            std::cout << "Distancia: error de lectura I2C" << std::endl;
        } else if (distance == 0) {
            std::cout << "Distancia: fuera de rango" << std::endl;
        } else {
            window.push_back(distance);
            if (window.size() > kWindowSize) {
                window.erase(window.begin());
            }
            std::vector<uint16_t> sorted(window);
            std::sort(sorted.begin(), sorted.end());
            std::cout << "Distancia: " << sorted[sorted.size() / 2]
                      << " mm (raw " << distance << ")" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << std::endl;
    std::cout << "Saliendo del programa... Liberando recursos." << std::endl;
    laser.reset();

    std::cout << "Memoria liberada correctamente." << std::endl;
    return 0;
}
