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

En Windows, usa `build.bat` como punto unico de entrada:

```powershell
.\build.bat debug
.\build.bat release
.\build.bat debug r2d_collect
.\build.bat release r2d_sandbox
.\build.bat debug all
```

El primer argumento es la configuracion (`debug` o `release`). El segundo argumento es
opcional: usa `all` por defecto o el nombre de un target CMake (`r2d_sandbox`,
`r2d_collect`, `r2d_sfx_editor`, `r2d_midi_player`). Para solo regenerar el proyecto:

```powershell
.\build.bat configure
```

Los scripts antiguos (`build_debug.bat`, `build_release.bat`, `build_sandbox_debug.bat`,
`build_sfx_editor_debug.bat` y `build_midi_player_debug.bat`) siguen existiendo como
wrappers temporales hacia `build.bat` para no romper flujos existentes. La build `Release`
enlaza las demos como aplicaciones Windows, asi que no abren consola.

Tambien puedes llamar a CMake directamente:

```powershell
cmake -S . -B build
cmake --build build --config Debug
cmake --build build --config Release --target r2d_collect
```

Si prefieres compilar con una ventana simple de un click:

```powershell
.\build_gui.bat
```

El launcher visual permite elegir `Debug` o `Release`, elegir un target, configurar,
compilar, y compilar/ejecutar. La lista sale de `tools/build_targets.json`; para anadir
un ejemplo o herramienta nueva, anade el target a CMake y una entrada a ese JSON.

## Ejecutar ejemplo

```powershell
.\build\r2d_sandbox.exe
```

Con Visual Studio/MSVC, el ejecutable queda en:

```powershell
.\build\Debug\r2d_sandbox.exe
.\build\Release\r2d_sandbox.exe
```

Tambien se incluye una mini demo jugable de recoger monedas:

```powershell
.\build\Debug\r2d_collect.exe
.\build\Release\r2d_collect.exe
```

Pulsa `C` en el sandbox para activar o desactivar el efecto CRT. Pulsa `R` para
recargar `assets/shaders/crt.fs` sin cerrar el programa.
Pulsa `F3` para ver overlays de depuracion del tilemap: tiles de colision, objetos y
viewport de camara.
Pulsa `Z`, `X`, `V`, `B`, `N` o `M` para probar presets generados con el sintetizador.
El sintetizador soporta envolvente, slide de tono, vibrato, arpegio, duty sweep y filtros
paso bajo, paso alto y paso banda por voz.
Los presets se cargan desde `assets/audio/sfx/*.r2sfx`; si un archivo falta, el sandbox usa
el preset compilado como fallback.
El jugador del sandbox usa una spritesheet procedural para probar `R2D_SpriteSheet`,
`R2D_AnimPlayer` y el dibujado de animaciones sin depender de assets externos.
Tambien carga `assets/tilemaps/r2d_sandbox.json`, un mapa Tiled JSON minimo incluido con
el framework. Si el mapa tiene un objeto llamado `PlayerStart`, el sandbox usa su posicion
como spawn del jugador.
La camara del sandbox sigue al jugador y se limita al rectangulo del mapa.
El dibujado del sandbox usa el rectangulo visible de la camara para no recorrer todo el
tilemap en mapas grandes.

## Demo collect

`examples/collect` es una mini demo jugable, separada del sandbox, que carga
`assets/tilemaps/collect.json`. Usa spritesheets reales para el jugador y las monedas,
musica `.r2song`, SFX al recoger pickups, camara con clamp y CRT fijo.

Controles:

- `WASD` o flechas: mover el jugador.
- `F3`: mostrar u ocultar debug de colision y objetos.

Convenciones del mapa Tiled:

- Objeto `PlayerStart`: posicion inicial del jugador.
- Objetos `type=coin`, `type=pickup` o nombre que empiece por `Coin`: monedas recogibles.
- Capa `Collision`: colision invisible; cualquier tile no cero bloquea.
- Capas `Foreground`, `Above` u `Over`: se dibujan por encima del jugador.
- Resto de capas tile: se dibujan por debajo del jugador.

Al recoger todas las monedas, la demo muestra un mensaje `ALL CLEAR`.

El framework tambien puede reproducir musica MIDI con SoundFont usando TinySoundFont y
TinyMidiLoader. Coloca un MIDI en `assets/audio/music/theme.mid` y una SoundFont en
`assets/audio/soundfonts/chiptune.sf2`; el sandbox los carga si existen y permite activar
o parar la musica con `P`.

## Reproductor MIDI

```powershell
.\build.bat debug r2d_midi_player
.\build\Debug\r2d_midi_player.exe
```

El reproductor descubre todos los `.mid` y `.midi` de `assets/audio/music` y todos los
`.sf2` de `assets/audio/soundfonts`. Pulsa `F1` dentro del reproductor para ver todos
los controles. Las listas aceptan raton: click selecciona, doble click reproduce y la
rueda cambia la seleccion. Pulsa `Tab` para alternar una vista de canales MIDI; desde ahi
se pueden mutear canales y ajustar volumen o programa/instrumento por canal. `F6` guarda una configuracion `.r2song`
junto al MIDI cargado y `F7` carga la configuracion del MIDI seleccionado si existe. Esa
configuracion conserva SoundFont, loop, volumen global, mute, volumen, banco e instrumento por canal.
La linea de estado muestra si la cancion esta sin guardar, guardada o con cambios pendientes.
Los juegos pueden cargar una configuracion directamente con `R2D_MusicLoadSong()`, usando
una ruta como `R2D_AssetPath("audio/music/theme.r2song")`.

### Formato `.r2song`

Los `.r2song` son texto plano `clave=valor`. Van normalmente junto al MIDI en
`assets/audio/music` y describen como debe sonar esa cancion sin modificar el archivo MIDI.
Las rutas simples de `midi` se resuelven junto al `.r2song`; las rutas simples de
`soundfont` se buscan en `assets/audio/soundfonts`.

```ini
version=1
midi=theme.mid
soundfont=chiptune.sf2
loop=true
volume=0.65

channel0_muted=false
channel0_volume=1
channel0_bank=0
channel0_program=80

channel9_muted=false
channel9_volume=0.8
channel9_bank=128
channel9_program=0
```

Los canales se numeran de `0` a `15`; el canal `9` corresponde al canal MIDI 10, usado
habitualmente para percusion. Las claves de canal son opcionales: si faltan, el framework
usa volumen `1`, banco `0` para canales melodicos, banco `128` para percusion y el programa
indicado por el propio MIDI.

## Tilemaps Tiled

El soporte inicial de Tiled esta pensado para ser pequeno y predecible. Carga mapas JSON
ortogonales y finitos, con datos de tiles en array JSON sin compresion. Soporta tilesets
incrustados con imagen unica y tilesets externos `.tsx` simples con `<image source="...">`.

Las capas `tilelayer` se guardan como capas de tiles. Una capa llamada `Collision` puede
usarse para colision por tiles: cualquier valor distinto de cero bloquea. Las capas
`objectgroup` cargan objetos rectangulares con `name`, `type`, `x`, `y`, `width` y `height`;
el sandbox usa `PlayerStart` como punto de spawn.

Quedan fuera a proposito, de momento: mapas infinitos, chunks, base64, compresion,
isometrico/hexagonal, propiedades custom, tiles animados y multiples tilesets complejos.
La idea es usar Tiled como editor potente sin convertir el framework en un motor enorme.

## Editor de efectos

```powershell
.\build.bat debug r2d_sfx_editor
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

En `Debug`, el codigo resuelve rutas con `R2D_AssetPath("shaders/crt.fs")` y primero
intenta leer la carpeta `assets` del proyecto, para que editar shaders, mapas o presets
en runtime sea directo. En `Release`, los ejemplos empaquetan solo los assets que usa
cada target en un archivo junto al ejecutable: `r2d_collect.assets`,
`r2d_sandbox.assets`, etc. El framework monta automaticamente el `.assets` con el mismo
nombre que el `.exe` y, si no existe, cae al formato clasico `assets` junto al
ejecutable.

El empaquetado se controla con `R2D_PACKAGE_ASSETS` y esta activado por defecto para los
ejemplos. Las herramientas de desarrollo siguen copiando la carpeta `assets`, porque
necesitan listar directorios y guardar archivos editables.

## Estructura

```text
assets                  Recursos que se copian junto al ejecutable
*.assets                Paquete de assets por ejecutable en Release
assets/audio/sfx        Presets de sintetizador .r2sfx
assets/audio/music      Canciones MIDI
assets/audio/soundfonts Bancos SoundFont .sf2
external/tinysoundfont  TinySoundFont y TinyMidiLoader
include/r2d/r2d.h       API publica
src/r2d.c               Implementacion del framework
src/r2d_assets.c        Carga desde carpeta o paquete .assets
src/r2d_camera.c        Camara 2D simple para coordenadas de mundo y pantalla
src/r2d_crt.c           Postproceso CRT opcional
src/r2d_audio.c         Sintetizador simple para efectos retro
src/r2d_music.c         Reproduccion MIDI + SoundFont
src/r2d_sprite.c        Spritesheets en grid y animacion simple
src/r2d_tilemap.c       Carga y dibujado basico de mapas Tiled JSON
examples/sandbox        Primer juego de prueba
examples/collect        Mini demo jugable de recoger monedas
tools/sfx_editor        Editor sencillo de presets de sonido
tools/midi_player       Reproductor para probar MIDIs con SoundFonts
tools/build_gui.ps1     Launcher visual simple para builds
tools/build_targets.json Targets visibles en el launcher
```
