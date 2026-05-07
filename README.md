# Retro2DFramework

Un framework C pequeño sobre raylib para juegos retro 2D.

Objetivos iniciales:

- Resolucion virtual con escalado pixel-perfect.
- Bucle de juego simple basado en callbacks.
- Renderizado a textura para mantener pixeles nitidos.
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
```

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

## Estructura

```text
include/r2d/r2d.h       API publica
src/r2d.c               Implementacion del framework
examples/sandbox        Primer juego de prueba
```
