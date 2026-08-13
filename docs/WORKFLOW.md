# Workflow de Desarrollo

Flujo de trabajo establecido para el proyecto VL53L0X.

## Principios

- **Cambios de codigo:** se hacen exclusivamente a nivel **local**, en `$PWD`
  (el repo local). Nada se edita directo en la maquina remota.
- **Compilacion:** se realiza **solo de forma remota**, en la Raspberry Pi.
- **Pruebas:** se ejecutan **solo de forma remota**, en la Raspberry Pi.
- **Generacion y modificacion:** todo archivo generado o modificado vive en
  `$PWD` local. No se dejan archivos generados en el remoto.

## Flujo paso a paso

1. **Editar local:** realizar los cambios de codigo/documentacion en `$PWD`.
2. **Commit local:** commitear con mensajes semanticos
   (`feat:`, `fix:`, `chore:`, `docs:`, etc.).
3. **Push:** pushear los commits a `origin/main`.
4. **Actualizar remoto:** en la Pi, `git pull` para traer los cambios.
5. **Compilar remoto:** build en la Pi.
6. **Probar remoto:** ejecutar las pruebas en la Pi.

## Comandos de referencia

```bash
# Edicion local (en $PWD)
vim src/vl53l0x/VL53L0X_impl.cpp

# Commit y push local
git add <archivos>
git commit -m "fix: ..."
git push origin main

# Bump de version y tag (cuando corresponda)
echo "1.1.9" > VERSION
git add VERSION && git commit -m "chore: bump version to 1.1.9"
git tag v1.1.9
git push origin main && git push origin v1.1.9

# Actualizar, compilar y probar en la Pi (solo remoto)
sshpass -e ssh -o StrictHostKeyChecking=no pi@raspi.local \
  "cd /home/pi/src/vl53l0x_rpi && git pull --ff-only && make clean && make -j4 && make run"
```

## Version de la app

- La version vive en el archivo `VERSION` (raiz del proyecto).
- El Makefile la inyecta en el build con `-DAPP_VERSION`.
- Se muestra en el banner de presentacion de la app.
- Cada bump va acompanado de un commit `chore: bump version to X.Y.Z` y un
  tag `vX.Y.Z` pusheado al remoto.

## Recordatorios

- No compilar ni probar en la maquina local.
- No editar archivos dentro del repo remoto de la Pi; solo `git pull` y build.
- La password de la Pi va en `$SSHPASS`; nunca mostrarla en pantalla.
