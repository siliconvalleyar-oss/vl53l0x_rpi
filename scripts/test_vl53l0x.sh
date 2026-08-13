#!/bin/bash
# test_vl53l0x.sh - Prueba simple del modulo VL53L0X por I2C directo (i2c-tools)
# Detecta el modulo, lo inicializa y toma N mediciones mostrando el valor crudo.
#
# Uso: sudo bash scripts/test_vl53l0x.sh [BUS] [DIR] [N_MEDICIONES]
#
# Nota: usar la via del kernel (i2c-tools) evita el problema del bus colgado
# que deja la app con la libreria bcm2835 en los clones del VL53L0X.

BUS=${1:-1}
DIR=${2:-0x29}
N=${3:-5}
I2CGET=/usr/sbin/i2cget
I2CSET=/usr/sbin/i2cset

if [ ! -x "$I2CGET" ] || [ ! -x "$I2CSET" ]; then
    echo "Error: no se encontro i2cget/i2cset (instalar i2c-tools)" >&2
    exit 1
fi

setbyte() { timeout 3 "$I2CSET" -y "$BUS" "$DIR" "$1" "$2" >/dev/null 2>&1 || echo "  ERROR escribiendo reg $1"; }
getbyte() { timeout 3 "$I2CGET" -y "$BUS" "$DIR" "$1" 2>/dev/null; }
h2d() { local v="$1"; [ -z "$v" ] && v=0xFF; v=$(echo "$v" | tr -d '\n'); echo $((v)); }

echo "=== VL53L0X en bus $BUS dir $DIR ==="
m=$(getbyte 0xC0); m=$(h2d "$m")
printf "Model ID (0xC0): 0x%02X\n" "$m"
[ "$m" -eq 238 ] && echo "  ID OK (0xEE)" || echo "  OJO: ID distinto de 0xEE"

echo "=== Wake (DataInit) ==="
setbyte 0x88 0x00; setbyte 0x80 0x01; setbyte 0xFF 0x01; setbyte 0x00 0x00
getbyte 0x91 >/dev/null
sleep 0.1
setbyte 0x00 0x01; setbyte 0xFF 0x00; setbyte 0x80 0x00

echo "=== StaticInit (SSC) ==="
setbyte 0xFF 0x01; setbyte 0x00 0x00; setbyte 0xFF 0x06
a=$(getbyte 0x83); a=$(h2d "$a")
setbyte 0x83 $((a | 0x04)); setbyte 0xFF 0x07; setbyte 0x81 0x01; setbyte 0x80 0x01
setbyte 0x94 0x6B; setbyte 0x83 0x00
ok=0
for i in $(seq 1 100); do
    s=$(getbyte 0x83); s=$(h2d "$s")
    [ $((s & 1)) -eq 1 ] && { ok=1; break; }
    sleep 0.01
done
echo "  SSC completo: $ok"
setbyte 0x83 0x00; setbyte 0xFF 0x06; setbyte 0x83 $((a & ~0x04)); setbyte 0xFF 0x01
setbyte 0x00 0x01; setbyte 0xFF 0x00; setbyte 0x80 0x00

setbyte 0x0A 0x04; setbyte 0x0B 0x01; setbyte 0x94 0xE8

echo "=== Mediciones (${N}x) ==="
for n in $(seq 1 "$N"); do
    setbyte 0x00 0x01
    ready=0
    for i in $(seq 1 50); do
        st=$(getbyte 0x13); st=$(h2d "$st")
        [ $((st & 0x07)) -ne 0 ] && { ready=1; break; }
        sleep 0.002
    done
    rs=$(getbyte 0x14); rs=$(h2d "$rs")
    hi=$(getbyte 0x1E); hi=$(h2d "$hi")
    lo=$(getbyte 0x1F); lo=$(h2d "$lo")
    dist=$(( (hi << 8) | lo ))
    est=$(( (rs >> 3) & 0x0F ))
    printf "  medida %d: ready=%s estado_0x14=0x%02X raw=%d mm\n" "$n" "$ready" "$rs" "$dist"
    setbyte 0x0B 0x01
    sleep 0.3
done
echo "=== Fin ==="
