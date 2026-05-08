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
`build_sfx_editor_debug.bat` y `build_midi_player_debug.bat` compilan solo el juego de
prueba, el editor de efectos o el reproductor MIDI. La build `Release` enlaza el ejemplo
como aplicacion Windows, asi que no abre consola.

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

El framework tambien puede reproducir musica MIDI con SoundFont usando TinySoundFont y
TinyMidiLoader. Coloca un MIDI en `assets/audio/music/theme.mid` y una SoundFont en
`assets/audio/soundfonts/chiptune.sf2`; el sandbox los carga si existen y permite activar
o parar la musica con `P`.

## Reproductor MIDI

```powershell
.\build_midi_player_debug.bat
.\build\Debug\r2d_midi_player.exe
```

El reproductor descubre todos los `.mid` y `.midi` de `assets/audio/music` y todos los
`.sf2` de `assets/audio/soundfonts`. Usa `Izquierda` / `Derecha` para cambiar entre la
lista de MIDIs y la lista de SoundFonts, `Arriba` / `Abajo` para seleccionar, `Espacio`
o `Enter` para reproducir, pausar o continuar, `S` para parar, `R` para reiniciar,
`L` para activar o desactivar loop, `A` para auto-play al cambiar seleccion, `-` / `+`
para volumen y `F5` para refrescar carpetas.

## Editor de efectos

```powershell
.\build_sfx_editor_debug.bat
.\build\Debug\r2d_sfx_editor.exe
```

Controles:

- `Arriba` / `Abajo`: elegir parametro.
- `Izquierda` / `Derecha`: cambiar valor.
- `Shift`: cambio rapido.
- `Raton`: seleccionar parametro; arrastrar barras numericas.
- Click en las flechas superiores: cambiar preset.
- `Q` / `E`: cambiar preset.
- `Espacio`: reproducir.
- `A`: activar o desactivar auto-play al editar.
- `R`: mutar el preset actual con cambios pequenos.
- `N`: crear una variante numerada del preset actual.
- `C`: clonar el sonido actual en `editor.r2sfx`.
- `F5`: refrescar la carpeta `assets/audio/sfx`.
- `S`: guardar el preset actual.
- `L`: recargar el preset actual desde disco.
- `Backspace`: restaurar un sonido base.
- `Ctrl+Z` / `Ctrl+Y`: deshacer y rehacer cambios.

Los archivos `.r2sfx` son texto plano `clave=valor`, pensados para versionarse y editarse a
mano si hace falta. El editor muestra un preview simple de envolvente, tono y filtro. Un
asterisco junto al preset indica cambios pendientes de guardar; el editor no cambia de
preset mientras haya cambios sin guardar.

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
assets/audio/music      Canciones MIDI
assets/audio/soundfonts Bancos SoundFont .sf2
external/tinysoundfont  TinySoundFont y TinyMidiLoader
include/r2d/r2d.h       API publica
src/r2d.c               Implementacion del framework
src/r2d_crt.c           Postproceso CRT opcional
src/r2d_audio.c         Sintetizador simple para efectos retro
src/r2d_music.c         Reproduccion MIDI + SoundFont
examples/sandbox        Primer juego de prueba
tools/sfx_editor        Editor sencillo de presets de sonido
tools/midi_player       Reproductor para probar MIDIs con SoundFonts
```
