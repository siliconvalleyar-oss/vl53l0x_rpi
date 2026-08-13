# Reporte de análisis — VL53L0X siempre lee "fuera de rango"

**Fecha:** 2026-08-13
**Síntoma reportado:** El sensor se inicializa correctamente (el ID `0xEE` se lee en `0xC0`), el bus I2C responde, pero `medir()` devuelve siempre `0` / `0xFFFF` y el programa imprime "fuera de rango".

**Conclusión general:** El I2C funciona, pero **la inicialización del sensor es incompleta/incorrecta** y **el código no valida el estado real de la medición**, por lo que un fallo de medición o de lectura se interpreta como "fuera de rango". A continuación se listan las fallas identificadas, ordenadas por severidad.

---

## REGLA DE LECTURA (obligatoria)

> **Quien lea este documento debe marcar con `[X]` cada ítem del checklist una vez leído.**
> Ninguna sección debe darse por conocida sin marcar su casilla. Si se relee, se confirma la marca.
> El lector firma al final (`[Lectura verificada por: __________]`).

### Checklist de lectura

- [ ] Conclusión general
- [ ] Sección 0 — Evidencia observada (raw=0)
- [ ] Sección 1 — Inicialización incompleta del sensor
- [ ] Sección 2 — `read_distance()` ignora timeout/estado
- [ ] Sección 3 — Errores I2C convertidos en 0
- [ ] Sección 4 — `calculate_distance()` enmascara fallos
- [ ] Sección 5 — Conflicto de registro 0x1E
- [ ] Sección 6 — `set_mode()` no-op; modo continuo roto
- [ ] Sección 7 — Flag de init corrupto en `bcm2835_init`
- [ ] Sección 8 — Otros problemas menores
- [ ] Diagnóstico recomendado (próximos pasos)
- [ ] Referencias cruzadas

[Lectura verificada por: ______________ Fecha: ______________]

---

## 0. EVIDENCIA OBSERVADA (salida real del programa)

```
Debug: high=0 low=0 raw=0     (repetido en cada iteración)
Distancia: fuera de rango
```

### Lectura de la evidencia

El dato más importante es **`raw = 0x0000`**, NO `0xFFFF`. En el VL53L0X:

| Valor en los registros de resultado | Significado |
|---|---|
| `0xFFFF` | La medición **se ejecutó** pero falló (sin objetivo / señal insuficiente) |
| `0x0000` | Los registros de resultado **nunca se poblaron**: la medición **no llegó a ejecutarse/completarse** (o la lectura I2C falló) |

Como `raw = 0` de forma consistente, la medición **no se está completando** en ninguna iteración, o bien **toda lectura I2C está fallando silenciosamente** (recordar que `read_register()` devuelve `0` ante cualquier error I2C — ver punto 3). El valor `0xFFFF` típico de "medición ejecutada sin resultado" nunca aparece.

Además, cada ciclo tarda ~700 ms en imprimir (200 ms de timeout del poll `0x13` + 500 ms de `sleep_for` en `main.cpp:52`), lo que sugiere que **el poll de `VL53L0X_REG_RESULT_INTERRUPT_STATUS` (0x13) expira siempre**: el bit 0 nunca se activa.

### Dos hipótesis para discriminar

- **Hipótesis A — la medición nunca arranca/completa:** el init dejó el sensor sin calibrar/incorrectamente configurado; al escribir `0x01` en `0x00` la medición no se ejecuta, los registros de resultado quedan en `0` y el bit de interrupción nunca se pone. **Es la más probable** dado que el init es recortado (punto 1).
- **Hipótesis B — el I2C se rompe después del init:** si `bcm2835_i2c_write/read` empieza a fallar (NACK, clock stretching, baudrate), `read_register()` devuelve `0` silenciosamente, incluyendo el poll de `0x13` y las lecturas de `0x1E/0x1F`. `raw=0` sería el artefacto de ese fallo, no un dato real del sensor.

**Prueba rápida para distinguirlas:** dentro de `read_distance()` leer también `0xC0` (`MODEL_ID`) junto a `0x1E/0x1F`. Si `0xC0` sigue devolviendo `0xEE`, el I2C sigue vivo → **Hipótesis A** (el sensor no mide). Si `0xC0` devuelve `0`, el I2C se rompió → **Hipótesis B**. Imprimir también `status` del poll y el valor de retorno de `write_reg(0x00, 0x01)`.

---

## 1. CRÍTICA — Inicialización incompleta del sensor (causa más probable)

**Archivo:** `src/vl53l0x/VL53L0X_impl.cpp:64-165`

`initialize()` solo verifica el `MODEL_ID` (`0xEE` en `0xC0`, líneas 78-81) y luego escribe una versión **recortada** de la secuencia de calibración de referencia de ST. Si el ID se lee bien, la inicialización se da por exitosa, aunque el sensor no quede listo para medir.

Fallos concretos de la secuencia:

1. **Falta la calibración de referencia/SPAD.** La API oficial de ST ejecuta `VL53L0X_PerformRefCalibration()` (y opcionalmente `PerformRefSpadManagement`) después de la secuencia de registros. Sin esto, el síntoma clásico es exactamente el reportado: lecturas `0xFFFF` (out of range) siempre.
2. **Valores de registros distintos a la secuencia de referencia de ST** (por ejemplo `0x30=0x01` vs `0x09` del driver ST, `0x54=0xFF` vs `0x00`, `0x32=0x00` vs `0x03`, `0x57=0x00` vs `0x30`, `0x30` (página 1) `=0x00` vs `0x20`). Esto deja el *timing budget* y el *signal rate* mal configurados.
3. **Faltan muchas escrituras de *tuning* de ST** (`0x40`, `0x31`, `0x44`, `0x45`, `0x46`, `0x67`, `0x70`, `0x71`, `0x72`, `0x76`, `0x77`, etc.).

**Impacto:** la medición nunca completa correctamente → los registros de resultado quedan en `0xFFFF`.

**Fix sugerido:** usar la secuencia completa de ST (`VL53L0X_DataInit` + `VL53L0X_StaticInit` + `VL53L0X_DefaultTuningSettings`) o portar la librería Pololu/ST y ejecutar la calibración de referencia antes de la primera medición.

---

## 2. CRÍTICA — `read_distance()` ignora si la medición falló o expiró

**Archivo:** `src/vl53l0x/VL53L0X_impl.cpp:167-190`

```cpp
for (int i = 0; i < 100; ++i) {
    uint8_t status = read_reg(VL53L0X_REG_RESULT_INTERRUPT_STATUS); // 0x13
    if (status & 0x01) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}
write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
uint8_t high = read_reg(VL53L0X_REG_RESULT_RANGE_STATUS + 10); // 0x1E
uint8_t low  = read_reg(VL53L0X_REG_RESULT_RANGE_STATUS + 11); // 0x1F
```

- **Sin manejo de timeout:** si el bucle de 100×2 ms expira, se leen los registros de resultado igualmente → `raw = 0xFFFF` → "fuera de rango".
- **No se valida el byte `RESULT_RANGE_STATUS` (0x14):** nunca se comprueban los bits de error / *device busy* / *out of range* del propio resultado. Un resultado inválido no se distingue de uno válido.
- **No se comprueba que el bit de START se haya limpiado** (`0x00` bit 0). La mayoría de drivers esperan primero a que el bit de arranque se borre y luego a `RESULT_INTERRUPT_STATUS & 0x07` (los 3 bits bajos, no solo el bit 0).
- **El bit de interrupción (0x13 bit 0) puede no activarse nunca** si la config de interrupción (0x0A) no quedó bien puesta tras el init defectuoso → el poll siempre expira.

**Impacto:** cualquier fallo de la medición se convierte silenciosamente en "fuera de rango".

---

## 3. ALTA — Errores de I2C convertidos en "0 mm"

**Archivo:** `src/lib/bcm2835_wrapper.cpp:27-38`

```cpp
uint8_t I2cDevice::read_register(uint8_t reg) {
    if (!initialized_) return 0;
    if (bcm2835_i2c_write(...) != BCM2835_I2C_REASON_OK) return 0;
    if (bcm2835_i2c_read(...) != BCM2835_I2C_REASON_OK) return 0;
    return static_cast<uint8_t>(buf[0]);
}
```

- Ante cualquier fallo de I2C (NACK, *clock stretching*, etc.) devuelve `0` sin señal de error. En `read_distance()` eso produce `raw = 0x0000` → `distance = 0` → "fuera de rango".
- Es imposible distinguir "no hay objetivo" de "falló la lectura".
- La lectura de un byte se hace en dos transacciones (`write` del puntero de registro y luego `read`); entre ambas se emite STOP. Funciona en VL53L0X porque retiene el puntero, pero es frágil y sin reintentos.
- No hay reintentos ni *error handling* en toda la cadena `read_distance()`.

**Fix sugerido:** propagar un código de error (o devolver 0xFFFF/estado) y registrar el motivo; idealmente usar una lectura combinada (repetid *start*) del bloque `0x14..0x1F`.

---

## 4. ALTA — `calculate_distance()` enmascara los fallos

**Archivo:** `src/vl53l0x/VL53L0X_impl.cpp:247-253`

```cpp
if (raw == 0 || raw == 0xFFFF) return 0;
```

`0` y `0xFFFF` se mapean ambos a "fuera de rango", pero:
- `0xFFFF` es el valor típico de resultado **inválido** (medición fallida).
- `0` es lo que devuelven las lecturas I2C **fallidas**.

Además, el clamp a `VL53L0X_MIN_DISTANCE` (30) y `VL53L0X_MAX_DISTANCE` (2000) hace que una distancia real de, por ejemplo, 1500 mm se reporte siempre como 2000 en `main.cpp` (que solo imprime si `0 < distance < 2000`).

---

## 5. MEDIA — Conflicto de registros: `0x1E`

**Archivo:** `include/vl53l0x/VL53L0X_regs.hpp:27` y `src/vl53l0x/VL53L0X_impl.cpp:182-183, 192-197`

- `VL53L0X_REG_RESULT_OFFSET = 0x1E` (línea 27).
- El byte alto de la distancia se lee de `0x14 + 10 = 0x1E` (línea 182).
- `apply_offset()` **escribe** en `0x1E` (línea 195).

El registro de *offset* real del VL53L0X es `0x30` (`ALGO_PART_TO_PART_RANGE_OFFSET_MM`), no `0x1E`. Si se llama a `calibrar(offset != 0)`, se corrompe el byte alto del resultado de distancia. En `main.cpp` se llama con `0`, así que hoy no dispara, pero es una bomba de tiempo.

---

## 6. MEDIA — `set_mode()` no hace nada útil y `read_distance()` ignora el modo

**Archivo:** `src/vl53l0x/VL53L0X_impl.cpp:199-212` y `:170`

```cpp
case MeasurementMode::SINGLE_SHOT:
    write_reg(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT); // 0x00 -> 0x00
```

- `SYSRANGE_START` = `0x00` y `MODE_SINGLESHOT` = `0x00`: escribir `0x00` en `0x00` es un *no-op* (no arranca nada). El arranque real lo hace `read_distance()` con `write_reg(0x00, 0x01)`.
- `read_distance()` **siempre arranca single-shot** (`0x01`), ignorando `mode_`. El modo `CONTINUOUS`/`TIMED` está roto.

---

## 7. MEDIA — `Bcm2835Init` queda marcado como inicializado aunque falle

**Archivo:** `include/lib/bcm2835_init.hpp:13-18`

```cpp
static std::atomic<bool> initialized{false};
if (!initialized.exchange(true)) {
    if (!bcm2835_init()) { throw std::runtime_error(...); }
}
```

Si `bcm2835_init()` lanza, el flag `initialized` ya quedó en `true`, así que cualquier intento posterior (p. ej. al crear `I2cDevice` tras fallar un `GpioPin`) se salta la inicialización real y continúa sobre una librería no inicializada.

---

## 8. BAJA — Otros problemas menores

- **`main.cpp:44-50`:** la lógica de reporte mezcla tres casos ("fuera de rango", "error", válido) pero `medir()` nunca devuelve un código de error; solo `0`. No hay forma de distinguir los casos.
- **`VL53L0X_impl.cpp:51-62`:** `wait_for_device()` no se usa; el ID se lee una sola vez sin reintentos ni espera tras el reset por XSHUT.
- **`VL53L0X.cpp:72-82`:** `get_address()`, `get_offset()`, `is_initialized()` son stubs que devuelven valores fijos/falsos.
- **`gpio_wrapper.cpp:23-27`:** `get_state()` devuelve `PIN_LOW` si no está inicializado (indistinguible de un nivel real bajo).
- **`main.cpp:30`:** `calibrar(0)` no calibra nada; no sustituye a `PerformRefCalibration`.
- **`Makefile`:** usa `wildcard $(SRC_DIR)/**/*.cpp` que en GNU Make no es recursivo; solo funciona porque los fuentes están a un nivel de profundidad.

---

## Diagnóstico recomendado (próximos pasos)

Dada la evidencia `raw=0`, el siguiente paso concreto es discriminar entre las hipótesis A y B de la sección 0:

1. **Probar si el I2C sigue vivo durante `medir()`.** En `read_distance()` leer `0xC0` (`MODEL_ID`) junto a `0x1E/0x1F` e imprimir los tres. `0xC0 = 0xEE` → el bus está bien y el sensor no mide (Hipótesis A). `0xC0 = 0` → el I2C falla (Hipótesis B).
2. **Verificar si el poll de 0x13 expira siempre.** Agregar una marca de timeout en `read_distance()` e imprimir el `status` leído. Si siempre expira, el problema es el init (punto 1) o la config de interrupción (0x0A).
3. **Comprobar el valor de retorno de `write_reg(0x00, 0x01)`** (arranque de la medición): `write_register()` devuelve `false` si la escritura falla, pero `read_distance()` lo ignora.
4. **Leer el byte de estado `0x14` (RESULT_RANGE_STATUS) en cada medición** y volcarlo: los bits de error / *device busy* de ese byte explican por qué el resultado es inválido.
5. **Comparar el dump completo de registros** con el de un driver conocido (Pololu/ST) tras el init: las diferencias señalan exactamente qué *tuning* falta.
6. **Aplicar la secuencia completa de ST o portar la librería Pololu** y ejecutar `PerformRefCalibration` antes de la primera lectura.

---

## Referencias cruzadas

| Problema | Ubicación | Severidad |
|---|---|---|
| Init incompleto / sin calibración de referencia | `VL53L0X_impl.cpp:64-165` | CRÍTICA |
| Sin validación de timeout/estado en medición | `VL53L0X_impl.cpp:167-190` | CRÍTICA |
| Errores I2C silenciados como `0` | `bcm2835_wrapper.cpp:27-38` | ALTA |
| `0xFFFF`/`0` tratados como out of range | `VL53L0X_impl.cpp:247-253` | ALTA |
| Conflicto de registro `0x1E` (offset vs resultado) | `VL53L0X_regs.hpp:27`, `VL53L0X_impl.cpp:182-195` | MEDIA |
| `set_mode()` no-op; modo continuo roto | `VL53L0X_impl.cpp:199-212`, `:170` | MEDIA |
| Flag de init global corrupto en error | `bcm2835_init.hpp:13-18` | MEDIA |
| Stubs falsos en la API | `VL53L0X.cpp:72-82` | BAJA |
