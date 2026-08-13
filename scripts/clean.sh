#!/bin/bash
set -e

echo "Limpiando proyecto VL53L0X..."

# Limpiar usando make
make clean

# Eliminar binarios adicionales
rm -f bin/*
rm -rf obj/*

echo "Limpieza completada!"
