#!/bin/bash
set -e

echo "Instalando dependencias para VL53L0X..."

# Actualizar paquetes
echo "Actualizando lista de paquetes..."
sudo apt update

# Instalar herramientas de compilacion
echo "Instalando herramientas de compilacion..."
sudo apt install -y build-essential git cmake pkg-config

# Instalar libreria bcm2835
echo "Instalando libreria bcm2835..."
if ! dpkg -l | grep -q bcm2835; then
    wget -q http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
    tar -zxvf bcm2835-1.71.tar.gz
    cd bcm2835-1.71
    ./configure
    make
    sudo make check
    sudo make install
    cd ..
    rm -rf bcm2835-1.71 bcm2835-1.71.tar.gz
    echo "bcm2835 instalado correctamente"
else
    echo "bcm2835 ya esta instalado"
fi

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
