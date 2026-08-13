#!/bin/bash
set -e

echo "Instalando dependencias para VL53L0X..."

# Actualizar paquetes
echo "Actualizando lista de paquetes..."
sudo apt update

# Instalar herramientas de compilacion
echo "Instalando herramientas de compilacion..."
sudo apt install -y build-essential git cmake pkg-config

# Control de pines por driver del kernel (pinctrl / raspi-gpio).
# pinctrl viene con raspberrypi-utils (nuevo); raspi-gpio con wiringpi (viejo).
# El script usa pinctrl si existe y cae a raspi-gpio.
echo "Instalando herramientas de control de pines..."
sudo apt install -y raspi-utils 2>/dev/null || sudo apt install -y pinctrl 2>/dev/null || true

# Instalar i2c-tools (para diagnosticos)
echo "Instalando i2c-tools..."
sudo apt install -y i2c-tools

# Habilitar I2C si no esta habilitado
echo "Verificando I2C..."
if ! grep -q "i2c-dev" /etc/modules; then
    echo "i2c-dev" | sudo tee -a /etc/modules
    echo "I2C habilitado. Reinicie el sistema."
fi

echo ""
echo "Dependencias instaladas correctamente!"
echo "Ahora ejecute: ./scripts/build.sh"
echo ""
echo "Nota: No olvide ejecutar los scripts con privilegios de superusuario cuando sea necesario."
