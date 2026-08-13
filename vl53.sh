#!/bin/bash
# vl53.sh - atajo a scripts/test_vl53l0x.sh (aplica la calibracion)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/scripts/test_vl53l0x.sh" "$@"
