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

**Filtro de glitch (20 mm):** si el resultado corregido es < 30 mm, la
medición se **reintenta una vez** (un nuevo single shot) antes de
reportar. Esto descarta el falso blanco intermitente (estado `0x41`)
que se da cuando el crosstalk del VCSEL supera la señal del objeto. Está
implementado en `read_distance()` (app, vía `measure_once()`) y en la
función `medir()` del script `test_vl53l0x.sh`.

## 5. Calibración por software (actual)

La calibración interna (SSC/SPAD) no completa en esta unidad, así que se
aplica una **ganancia lineal empírica** en `calculate_distance()`:

```
real_mm = raw_mm × 100 / kCalibRawToMmScale
```

Constantes actuales (`src/vl53l0x/VL53L0X_impl.cpp`):

```cpp
constexpr int32_t kCalibRawToMmOffset = 0;
constexpr int32_t kCalibRawToMmScale  = 120;  // (×100) → real = raw × 100 / 120
constexpr int32_t kMinValidDistanceMm = 30;   // debajo: fuera de rango
```

- Lecturas por debajo de 30 mm se muestran como «fuera de rango»
  (filtra glitches de 20 mm).
- La ganancia se calculó con la referencia **A4 = 297 mm**:
  raw estable medido = 358 mm → `100 × 297 / 358 ≈ 120` (v1.2.17).
  La anterior era 98 con raw = 290 mm; el módulo derivó al alza.

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
| F      | 358      | 120 |
| G      | 350–363  | 120 (sigue válida) |

La deriva de ±20 % entre sesiones es síntoma del VCSEL degradado. La
calibración por software centra el error, pero **no elimina el ruido ni
la deriva**: para mediciones precisas se recomienda reemplazar el módulo.
La sesión G (raw 350–363 → 291–302 corregidos, mediana ~296) confirmó que
la escala 120 quedó centrada en la referencia de 297 mm.

## 8. Diagnóstico rápido del módulo

| Síntoma | Posible causa | Acción |
|---------|---------------|--------|
| `raw = 8191` (0x1FFF) siempre | Sin señal / VCSEL no emite | Verificar reset XSHUT, alimentación y cableado |
| SSC nunca completa (`SSC completo: 0`) | VCSEL/SPAD degradado | Continuar con calibración empírica o reemplazar |
| `0x14 & 0x07 = 0x01` | VCSEL continuity test fail | Módulo defectuoso, reemplazar |
| Glitches de 20 mm (estado 0x41) | Crosstalk del VCSEL > señal del objeto | **Retry automático** en app (`measure_once()` en `read_distance`) y script (`medir()`): si el raw es < 30 mm se reintenta una vez |
| Ruido/deriva entre corridas | VCSEL degradado | Recalibrar (sección 6) o reemplazar |

## 9. Filtros de software (app)

Resumen de los filtros en la app (`src/main.cpp` y `VL53L0X_impl.cpp`)
para mitigar el ruido del módulo (v1.2.16 → v1.3.0):

| Filtro | Dónde | Parámetro | Efecto |
|--------|-------|-----------|--------|
| Retry de glitch de 20 mm | `read_distance()` (vía `measure_once()`) | 1 reintento | Descarta el falso blanco (estado `0x41`); si la 2ª medición también falla, reporta «fuera de rango» |
| Mínimo válido | `calculate_distance()` | `kMinValidDistanceMm = 30` | Lecturas < 30 mm → 0 («fuera de rango») |
| **Mediana móvil** | `main.cpp` (bucle de medición) | ventana de 9 lecturas | Estabiliza la salida: con la A4 fija a 297 mm muestra 295–298 |

- La mediana usa ventana de 9 (antes 5) para mayor estabilidad con objeto
  fijo; responde más lento a los movimientos.
- Cada lectura se muestra como `Distancia: X mm (raw Y)` (Y = valor crudo).

## 10. Scripts

- **`vl53.sh` (raíz)**: atajo a `scripts/test_vl53l0x.sh` (aplica la
  calibración). En la Pi había una **copia vieja** de este nombre que no
  aplicaba calibración y reportaba el raw (ej. 355 mm); se eliminó y se
  reemplazó por el atajo (commit 7eadd4e).
- **`scripts/test_vl53l0x.sh`**: aplica la misma calibración que la app:
  - Constantes `SCALE=120` y `MIN_VALID=30` al inicio.
  - Función `medir()`: deja `$DIST` (raw), `$RS` (estado) y `$MM`
    (calibrado; 0 = glitch/fuera de rango).
  - Retry de glitch basado en el valor calibrado (`$MM == 0`), igual que
    la app.
  - `i2cdetect` con ruta completa (`/usr/sbin/i2cdetect`) para shells no
    interactivos (commit 50f2091).
- **`scripts/install_deps.sh`**: ya no instala bcm2835 (no se usa — el
  I2C es por ioctl del kernel; `bcm2835_wrapper.cpp` solo tiene nombre
  engañoso). Instala `raspi-utils`/`pinctrl` y `i2c-tools`.
- **`scripts/run.sh`**: ejecuta la app; mensaje actualizado (ya no
  menciona bcm2835).

## 11. Versionado

Convención (ver `docs/WORKFLOW.md`):

- **fix** → patch (`1.2.16`, `1.2.17`).
- **feature** → minor (la mediana debió saltar a `1.3.0`, no `1.2.18`).
- Cada minor tiene exactamente 10 patches (0–9); luego minor.
- Tag = VERSION (con `v`). Ej.: tag `v1.3.0` → VERSION `1.3.0`.

Línea de tiempo reciente:

| Versión | Cambio |
|---------|--------|
| 1.2.15 | Recalibración escala 98 (raw 290 a 297 mm) |
| 1.2.16 | Fix: retry de glitch de 20 mm (app `measure_once` + script `medir`) |
| 1.2.17 | Recalibración escala 120 (raw 358 a 297 mm) |
| 1.3.0  | Feature: filtro de mediana móvil (ventana 9) + tag `v1.3.0` |
| 1.3.0  | Scripts: `vl53.sh` atajo, `i2cdetect` ruta completa, install_deps sin bcm2835 |

## 12. Acceso remoto a la Pi

- IP actual de la Raspberry Pi (hostname `raspberry`): `192.168.1.44`,
  usuario `joy`.
- Acceso por SSH con claves desde la máquina de desarrollo (ms7851).
- Repo en la Pi: `~/src/vl53l0x_rpi`.
- Para aplicar cambios: `git pull` en la Pi y recompilar (`make all`).

## 13. Referencias

- Script de prueba: `scripts/test_vl53l0x.sh` (mismo init que el C++).
- Reset de pines: `pinctrl` (kernel) — `raspi-gpio` como fallback.
- I2C por driver del kernel: `/dev/i2c-1` + ioctl (`I2C_RDWR`).
