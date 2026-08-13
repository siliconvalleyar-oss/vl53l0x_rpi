# Calibración y Ajustes del VL53L0X

Documento técnico de configuración, ajustes y calibración del sensor
VL53L0X en este proyecto (app C++ `laser_measure` y script `test_vl53l0x.sh`).

> **Advertencia**: esta unidad en particular tiene el VCSEL degradado
> (falla de continuidad: registro `0x14` bit 0 = 1) y nunca completa la
> calibración interna SSC/SPAD (`SSC completo: 0`). Por eso se usa una
> **calibración empírica por software** y las mediciones presentan
> ruido y deriva. Un módulo sano no debería necesitar esto.

## 1. Conexión (pines)

| VL53L0X | Raspberry Pi GPIO | Header | Configuración en código |
|---------|-------------------|--------|--------------------------|
| VIN     | 3.3 V             | Pin 1  | Alimentación (soporta 3.3–5 V) |
| GND     | GND               | Pin 6  | Tierra |
| SDA     | GPIO2             | Pin 3  | I2C bus `/dev/i2c-1` (100 kHz) |
| SCL     | GPIO3             | Pin 5  | I2C bus `/dev/i2c-1` (100 kHz) |
| XSHUT   | GPIO17            | Pin 11 | Salida: reset/wake (pinctrl) |
| GPIO1 (INT) | GPIO27        | Pin 13 | Entrada: interrupción (no usada) |

Dirección I2C: `0x29`.

Configuración en `src/main.cpp`:

```cpp
laser->set_xshut_pin(17);   // GPIO17 -> XSHUT
laser->set_gpio1_pin(27);   // GPIO27 -> GPIO1/INT
```

## 2. Control de pines (pinctrl)

Los pines se controlan con **pinctrl** (driver del kernel), igual que el
script de prueba. NO se usa la librería bcm2835 (eliminada en v1.2.14):
el acceso directo a `/dev/mem` dejaba al módulo en estados distintos en
cada corrida y podía colgar la Raspberry Pi.

Secuencia de reset de XSHUT (en `set_xshut_pin()` e `initialize()`):

```bash
pinctrl set 17 op      # salida
sleep 0.1
pinctrl set 17 dl      # LOW  (reset del módulo)
sleep 0.1
pinctrl set 17 dh      # HIGH (wake)
sleep 0.1
sleep 0.5              # espera de arranque del firmware
```

GPIO1/INT se configura como entrada: `pinctrl set 27 ip`.

## 3. Secuencia de inicialización (init mínimo)

La secuencia del C++ replica la del script `test_vl53l0x.sh` (que es la
que funciona):

1. **Reset XSHUT** (pinctrl, tiempos largos).
2. **DataInit (wake)**:
   `0x88=0x00`, `0x80=0x01`, `0xFF=0x01`, `0x00=0x00`,
   leer `0x91` (boot status), esperar 100 ms,
   `0x00=0x01`, `0xFF=0x00`, `0x80=0x00`.
3. **StaticInit (SSC)**:
   `0xFF=0x01`, `0x00=0x00`, `0xFF=0x06`, leer `0x83`,
   `0x83=0x83|0x04`, `0xFF=0x07`, `0x81=0x01`, `0x80=0x01`,
   `0x94=0x6B`, `0x83=0x00`, esperar bit 0 de `0x83` (100 × 10 ms).
   Restaurar: `0x83=0x00`, `0xFF=0x06`, `0x83=0x83&~0x04`,
   `0xFF=0x01`, `0x00=0x01`, `0xFF=0x00`, `0x80=0x00`.
4. **Config final**:
   `0x0A=0x04` (interrupción GPIO), `0x0B=0x01` (clear),
   `0x94=0xE8` (sequence config: TCC y MSRC desactivados).

No se escribe el bloque de tuning completo (~80 registros) del API de ST:
el script que funciona no lo usa y en esta unidad no aporta nada.

## 4. Medición

Por lectura de registros (single shot):

1. Iniciar: `0x00=0x01` (SYSRANGE_START).
2. Esperar listo: `0x13 & 0x07 != 0` (hasta 200 ms).
3. Leer distancia cruda: `0x1E` (high) y `0x1F` (low).
4. `raw = (0x1E << 8) | 0x1F`.
5. `raw = 0xFFFF` o `0x1FFF` (8191) → sin señal (fuera de rango).
6. Limpiar interrupción: `0x0B=0x01`.

## 5. Calibración por software (actual)

La calibración interna (SSC/SPAD) no completa en esta unidad, así que se
aplica una **ganancia lineal empírica** en `calculate_distance()`:

```
real_mm = raw_mm × 100 / kCalibRawToMmScale
```

Constantes actuales (`src/vl53l0x/VL53L0X_impl.cpp`):

```cpp
constexpr int32_t kCalibRawToMmOffset = 0;
constexpr int32_t kCalibRawToMmScale  = 98;   // (×100) → real = raw × 100 / 98
constexpr int32_t kMinValidDistanceMm = 30;   // debajo: fuera de rango
```

- Lecturas por debajo de 30 mm se muestran como «fuera de rango»
  (filtra glitches de 20 mm).
- La ganancia se calculó con la referencia **A4 = 297 mm**:
  raw estable medido = 290 mm → `297 / 290 = 1.024` → escala `98`.

## 6. Procedimiento de recalibración

El módulo **deriva con el tiempo** (ver sección 7), así que la ganancia
puede quedar desactualizada. Recalibrar así:

1. Colocar un objeto a una distancia conocida y estable (ej. hoja A4 =
   297 mm, sin regla).
2. Medir el valor crudo con el script (reset estable):
   ```bash
   sudo bash scripts/test_vl53l0x.sh 1 0x29 6
   ```
   Tomar el valor estable (ignorar glitches de 20 mm).
3. Calcular la escala:
   ```
   kCalibRawToMmScale = 100 × distancia_real_mm / raw_medido_mm
   ```
   Ejemplo: raw = 290 mm a 297 mm reales → `100 × 297 / 290 ≈ 98`.
4. Actualizar `kCalibRawToMmScale` en `VL53L0X_impl.cpp`, bumpear
   `VERSION`, compilar y verificar con `make run`.

## 7. Historial de deriva del módulo

Raw medido a la misma distancia (297 mm, A4) en distintas sesiones:

| Sesión | Raw (mm) | Escala resultante |
|--------|----------|-------------------|
| A      | 458      | 146 |
| B      | 372      | 114 |
| C      | 275      | 93  |
| D      | 241      | 81  |
| E      | 290      | 98  |

La deriva de ±20 % entre sesiones es síntoma del VCSEL degradado. La
calibración por software centra el error, pero **no elimina el ruido ni
la deriva**: para mediciones precisas se recomienda reemplazar el módulo.

## 8. Diagnóstico rápido del módulo

| Síntoma | Posible causa | Acción |
|---------|---------------|--------|
| `raw = 8191` (0x1FFF) siempre | Sin señal / VCSEL no emite | Verificar reset XSHUT, alimentación y cableado |
| SSC nunca completa (`SSC completo: 0`) | VCSEL/SPAD degradado | Continuar con calibración empírica o reemplazar |
| `0x14 & 0x07 = 0x01` | VCSEL continuity test fail | Módulo defectuoso, reemplazar |
| Glitches de 20 mm | Módulo inestable | Filtrados por `kMinValidDistanceMm` |
| Ruido/deriva entre corridas | VCSEL degradado | Recalibrar (sección 6) o reemplazar |

## 9. Referencias

- Script de prueba: `scripts/test_vl53l0x.sh` (mismo init que el C++).
- Reset de pines: `pinctrl` (kernel) — `raspi-gpio` como fallback.
- I2C por driver del kernel: `/dev/i2c-1` + ioctl (`I2C_RDWR`).
