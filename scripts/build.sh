#!/bin/bash
set -e

echo "Compilando proyecto VL53L0X..."

# Detectar arquitectura
ARCH=$(uname -m)
case $ARCH in
    armv7l)
        echo "Arquitectura detectada: 32-bit ARM (armv7l)"
        export CXXFLAGS="-march=armv7-a -mfloat-abi=hard -mfpu=neon"
        ;;
    aarch64)
        echo "Arquitectura detectada: 64-bit ARM (aarch64)"
        export CXXFLAGS="-march=armv8-a -mtune=cortex-a72"
        ;;
    *)
        echo "Arquitectura no reconocida: $ARCH"
        echo "Usando flags por defecto"
        export CXXFLAGS=""
        ;;
esac

# Compilar
make clean
make all

echo ""
echo "Compilacion exitosa!"
echo "Binario: bin/laser_measure"
echo ""
echo "Para ejecutar: sudo ./scripts/run.sh"
