#!/bin/bash
set -e

# Verificar que el binario existe
if [ ! -f "bin/laser_measure" ]; then
    echo "Error: binario no encontrado. Ejecute primero ./scripts/build.sh"
    exit 1
fi

# Verificar privilegios de superusuario
if [ "$EUID" -ne 0 ]; then
    echo "Advertencia: Este programa requiere privilegios de superusuario para acceder a pinctrl e I2C"
    echo "Reejecutando con sudo..."
    exec sudo "$0" "$@"
fi

echo "Iniciando sensor VL53L0X..."
echo "Presione Ctrl+C para salir"
echo ""

# Ejecutar binario
./bin/laser_measure
