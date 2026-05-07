# Retro2DFramework

Un framework C pequeño sobre raylib para juegos retro 2D.

Objetivos iniciales:

- Resolucion virtual con escalado pixel-perfect.
- Bucle de juego simple basado en callbacks.
- Renderizado a textura para mantener pixeles nitidos.
- Postproceso CRT opcional, inspirado en Nostalgika.
- Helpers para paletas, sprites y coordenadas virtuales.
- Ejemplos pequenos que sirvan como pruebas vivas del framework.

## Requisitos

- CMake 3.20 o superior.
- Un compilador C compatible con C99.
- raylib instalado, o acceso a red para que CMake descargue raylib 6.0 con `FetchContent`.

## Compilar

```powershell
cmake -S . -B build
cmake --build build
```

O en Windows:

```powershell
.\build_debug.bat
.\build_release.bat
.\build_sandbox_debug.bat
.\build_sfx_editor_debug.bat
```

`build_debug.bat` y `build_release.bat` compilan todo. Los scripts `build_sandbox_debug.bat`
y `build_sfx_editor_debug.bat` compilan solo el juego de prueba o solo el editor de efectos.
La build `Release` enlaza el ejemplo como aplicacion Windows, asi que no abre consola.

## Ejecutar ejemplo

```powershell
.\build\r2d_sandbox.exe
```

Con Visual Studio/MSVC, el ejecutable queda en:

```powershell
.\build\Debug\r2d_sandbox.exe
.\build\Release\r2d_sandbox.exe
```

Pulsa `C` en el sandbox para activar o desactivar el efecto CRT. Pulsa `R` para
recargar `assets/shaders/crt.fs` sin cerrar el programa.
Pulsa `Z`, `X`, `V`, `B`, `N` o `M` para probar presets generados con el sintetizador.
El sintetizador soporta envolvente, slide de tono, vibrato, arpegio, duty sweep y filtros
paso bajo, paso alto y paso banda por voz.
Los presets se cargan desde `assets/audio/sfx/*.r2sfx`; si un archivo falta, el sandbox usa
el preset compilado como fallback.

## Editor de efectos

```powershell
.\build_sfx_editor_debug.bat
.\build\Debug\r2d_sfx_editor.exe
```

Controles:

- `Arriba` / `Abajo`: elegir parametro.
- `Izquierda` / `Derecha`: cambiar valor.
- `Shift`: cambio rapido.
- `Espacio`: reproducir.
- `S`: guardar en `assets/audio/sfx/editor.r2sfx`.
- `L`: cargar `assets/audio/sfx/editor.r2sfx`.

Los archivos `.r2sfx` son texto plano `clave=valor`, pensados para versionarse y editarse a
mano si hace falta.

Atajos de ventana incluidos por el framework:

- `F11` o `Alt+Enter`: alternar pantalla completa.
- `F12` o `PrintScreen`: guardar captura en `screenshots` junto al ejecutable.

La carpeta `assets` se copia automaticamente junto al ejecutable al compilar. El codigo
resuelve rutas con `R2D_AssetPath("shaders/crt.fs")`, siempre relativas a esa carpeta
`assets` del ejecutable. En builds locales generadas por este CMake, primero intenta leer
la carpeta `assets` del proyecto para que editar shaders en runtime sea directo.

## Estructura

```text
assets                  Recursos que se copian junto al ejecutable
assets/audio/sfx        Presets de sintetizador .r2sfx
include/r2d/r2d.h       API publica
src/r2d.c               Implementacion del framework
src/r2d_crt.c           Postproceso CRT opcional
src/r2d_audio.c         Sintetizador simple para efectos retro
examples/sandbox        Primer juego de prueba
tools/sfx_editor        Editor sencillo de presets de sonido
```
