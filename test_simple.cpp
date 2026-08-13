#include <iostream>
#include <bcm2835.h>
#include <unistd.h>

#define VL53L0X_ADDRESS 0x29
#define PIN_XSHUT 17
#define PIN_GPIO1 4

uint8_t readByte(uint8_t reg) {
    bcm2835_i2c_write(&reg, 1);
    char buf[1];
    bcm2835_i2c_read(buf, 1);
    return buf[0];
}

void writeByte(uint8_t reg, uint8_t data) {
    char buffer[2] = {reg, data};
    bcm2835_i2c_write(buffer, 2);
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835." << std::endl;
        return 1;
    }

    bcm2835_i2c_begin();
    bcm2835_i2c_setSlaveAddress(VL53L0X_ADDRESS);
    bcm2835_i2c_set_baudrate(100000);

    bcm2835_gpio_fsel(PIN_XSHUT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(PIN_GPIO1, BCM2835_GPIO_FSEL_INPT);

    bcm2835_gpio_write(PIN_XSHUT, LOW);
    usleep(10000);
    bcm2835_gpio_write(PIN_XSHUT, HIGH);
    usleep(10000);

    uint8_t id = readByte(0xC0);
    std::cout << "ID del modelo: 0x" << std::hex << (int)id << std::dec << std::endl;
    if (id != 0xEE) {
        std::cerr << "ID del sensor incorrecto." << std::endl;
        bcm2835_i2c_end();
        bcm2835_close();
        return 1;
    }

    for (int i = 0; i < 10; ++i) {
        writeByte(0x00, 0x01);
        usleep(50000);

        uint8_t high = readByte(0x1E);
        uint8_t low = readByte(0x1F);
        uint16_t distance = (high << 8) | low;
        std::cout << "Distancia: " << distance << " mm" << std::endl;
        usleep(100000);
    }

    bcm2835_i2c_end();
    bcm2835_close();
    return 0;
}
