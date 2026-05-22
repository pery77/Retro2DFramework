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
.\build.bat release r2d_hello_index
.\build.bat debug r2d_collision_example
.\build.bat debug r2d_particle_example
.\build.bat debug r2d_palette_example
.\build.bat debug r2d_time_example
.\build.bat debug r2d_save_example
.\build.bat debug r2d_template_game
.\build.bat debug r2d_platformer_example
.\build.bat debug r2d_topdown_example
.\build.bat debug r2d_input_example
.\build.bat debug r2d_ui_example
.\build.bat debug all
.\build.bat release r2d_pack_game_r2d_collect
```

El primer argumento es la configuracion (`debug` o `release`). El segundo argumento es
opcional: usa `all` por defecto o el nombre de un target CMake. Los ejemplos principales
son `r2d_hello_index`, `r2d_input_example`, `r2d_ui_example`,
`r2d_audio_example`, `r2d_state_example`, `r2d_collision_example`,
`r2d_particle_example`, `r2d_palette_example`, `r2d_time_example`,
`r2d_save_example`, `r2d_template_game`, `r2d_platformer_example`,
`r2d_topdown_example` y `r2d_collect`.
Las herramientas son `r2d_sfx_editor` y `r2d_midi_player`. Para solo regenerar el proyecto:

```powershell
.\build.bat configure
```

La build `Release` enlaza las demos como aplicaciones Windows, asi que no abren consola.
Para crear una carpeta final distribuible de una demo, compila el target
`r2d_pack_game_<target>` en `Release`; por ejemplo:

```powershell
.\build.bat release r2d_pack_game_r2d_collect
```

La salida queda en `build/dist/Release/r2d_collect/`.

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

## Ejecutar Hello / index

```powershell
.\build\r2d_hello_index.exe
```

Con Visual Studio/MSVC, el ejecutable queda en:

```powershell
.\build\Debug\r2d_hello_index.exe
.\build\Release\r2d_hello_index.exe
```

`r2d_hello_index` es una pantalla pequena que lista los ejemplos vivos y apunta a que
target compilar para probar cada sistema.

## Ejemplos

Cada sistema tiene su propio ejemplo para que el codigo sea documentacion ejecutable:

```powershell
.\build\Debug\r2d_input_example.exe
.\build\Debug\r2d_ui_example.exe
.\build\Debug\r2d_audio_example.exe
.\build\Debug\r2d_state_example.exe
.\build\Debug\r2d_collision_example.exe
.\build\Debug\r2d_particle_example.exe
.\build\Debug\r2d_palette_example.exe
.\build\Debug\r2d_time_example.exe
.\build\Debug\r2d_save_example.exe
.\build\Debug\r2d_template_game.exe
.\build\Debug\r2d_platformer_example.exe
.\build\Debug\r2d_topdown_example.exe
.\build\Debug\r2d_collect.exe
```

Con Visual Studio/MSVC, las builds `Release` equivalentes quedan en `.\build\Release\`.

- `r2d_input_example`: acciones de entrada con teclado, raton y gamepad.
- `r2d_ui_example`: texto bitmap, typewriter, nine-slice, navegacion de menu y CRT.
- `r2d_audio_example`: presets `.r2sfx`, grupos de mixer y musica `.r2song` con MIDI + SoundFont.
- `r2d_state_example`: maquina de estados con push/pop para pausa.
- `r2d_collision_example`: objeto controlado por raton con solidos, triggers y sensores.
- `r2d_particle_example`: emisores, bursts y presets de particulas retro.
- `r2d_palette_example`: paletas, recolor de sprite, flash y fade por color.
- `r2d_time_example`: timers, tweens, shake, hitstop, slow motion, flash y fade.
- `r2d_save_example`: configuracion, progreso y high score persistidos.
- `r2d_template_game`: esqueleto limpio con input, estados, update/draw y shutdown.
- `r2d_platformer_example`: gravedad, salto, suelo y aterrizaje en plataforma.
- `r2d_topdown_example`: movimiento top-down, camara y `R2D_MoveAndSlide`.
- `r2d_collect`: mini demo jugable con tilemap, entidades, colision, camara, SFX y musica.

## Demo collect

`examples/collect` es una mini demo jugable, separada del Hello / index, que carga
`assets/tilemaps/collect.json`. Usa spritesheets reales para el jugador y las monedas,
musica `.r2song`, SFX al recoger pickups, camara con clamp y CRT fijo.

Controles:

- `WASD` o flechas: mover el jugador.
- `F3`: mostrar u ocultar debug de colision y objetos.

Convenciones del mapa Tiled:

- Objeto `PlayerStart`: posicion inicial del jugador.
- Objetos `type=coin`, `type=pickup` o nombre que empiece por `Coin`: monedas recogibles.
- Capa tile `Pickups` o capa con propiedad `spawn=coin`: cada tile no vacio crea una
  entidad moneda y la capa no se dibuja como tilemap.
- Capa `Collision`: colision invisible; cualquier tile no cero bloquea.
- Capas `Foreground`, `Above` u `Over`: se dibujan por encima del jugador.
- Resto de capas tile: se dibujan por debajo del jugador.

Al recoger todas las monedas, la demo muestra un mensaje `ALL CLEAR`.

El framework tambien puede reproducir musica MIDI con SoundFont usando TinySoundFont y
TinyMidiLoader. Coloca un MIDI en `assets/audio/music/theme.mid` y una SoundFont en
`assets/audio/soundfonts/chiptune.sf2`, o crea una configuracion `.r2song` para elegir
MIDI, SoundFont, loop, volumen y canales. `r2d_audio_example` carga una `.r2song` incluida
y permite activar o parar la musica con `P`.

El mixer separa el audio en grupos `R2D_AUDIO_GROUP_MUSIC`, `R2D_AUDIO_GROUP_SFX`,
`R2D_AUDIO_GROUP_UI` y `R2D_AUDIO_GROUP_AMBIENT`. Usa
`R2D_AudioSetGroupVolume()` para volumen instantaneo, `R2D_AudioFadeGroup()` junto a
`R2D_AudioMixerUpdate(dt)` para fades y `R2D_PlaySfxRandomPitch()` para que efectos muy
repetidos no suenen siempre identicos. `R2D_MusicSetGroup()` permite asociar una cancion
al grupo que corresponda. Para cambiar entre canciones, `R2D_MusicCrossfadeStart()` y
`R2D_MusicCrossfadeUpdate()` hacen un fundido de salida/entrada controlado por el juego.

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
`r2d_collect` usa `PlayerStart` como punto de spawn.

El loader tambien lee propiedades custom de Tiled en capas y objetos. Soporta propiedades
`string`, `int`, `float`, `bool` y `color`; se consultan con
`R2D_TilemapLayerFindProperty()` o `R2D_TilemapObjectFindProperty()` y los helpers
`R2D_TilemapPropertyString/Int/Float/Bool/Color()`. `r2d_collect` usa una propiedad
`debug_color` en la capa `Collision` para colorear el overlay de depuracion.

Los objetos con `type=trigger`, `type=sensor` o propiedad booleana `trigger=true` pueden
convertirse a colliders sensor con `R2D_TilemapTriggerColliders()`. El `user_data` del
collider apunta al `R2D_TilemapObject` original, para leer propiedades como `event`.
`r2d_collect` incluye un trigger de ejemplo que muestra un mensaje al pisarlo.

Las capas leen tambien `opacity`, `offsetx`, `offsety`, `parallaxx` y `parallaxy`. Los
dibujados existentes respetan opacidad y offset; para parallax de camara usa
`R2D_TilemapDrawLayerParallax()`, pasando el viewport de camara y la posicion de pantalla.
`r2d_collect` ya dibuja sus capas por esa ruta, asi que un mapa puede activar parallax
directamente desde Tiled.

Los tilesets pueden incluir tiles animados de Tiled, tanto en tilesets JSON incrustados
como en `.tsx` externos simples. El renderer cambia automaticamente al frame activo usando
las duraciones del tileset; no hace falta llamar a un update separado para que los tiles
animados avancen.

Los tilesets de imagen unica respetan `margin` y `spacing`, tambien en JSON incrustado y
`.tsx` externo. Esto permite usar atlas exportados por Tiled con separacion entre tiles
sin que el renderer lea pixels de borde equivocados.

## Colision 2D

La base de colision se apoya en los tipos y helpers de raylib. Retro2D usa `Rectangle`,
`Vector2` y funciones como `CheckCollisionRecs()`, `CheckCollisionPointRec()` y
`CheckCollisionCircleRec()` como fuente de verdad; encima anade colliders pequenos con
capa, mascara, flag `trigger` y `user_data`.

`R2D_CollisionQueryRect()`, `R2D_CollisionQueryPoint()` y `R2D_CollisionQueryCircle()`
devuelven overlaps filtrados por capa/mascara; los triggers se reportan como hits, pero
no bloquean movimiento. Para movimiento de personajes, `R2D_MoveAndSlide()` resuelve un
AABB contra una lista de colliders solidos eje por eje y devuelve la nueva posicion del
rectangulo.

Los tilemaps tienen helpers directos para la convencion `Collision`:
`R2D_TilemapCollisionRects()` convierte los tiles solidos de un area en colliders, y
`R2D_TilemapMoveAndSlide()` mueve un AABB contra una capa de tiles. `r2d_collect` usa este
helper para el movimiento del jugador.

`r2d_collision_example` oculta el cursor del sistema con `HideCursor()` y dibuja un objeto
propio siguiendo el raton virtual. Ese objeto se desliza contra solidos y muestra triggers,
sensores de punto y sensores de circulo en tiempo real.

## Grid y pathfinding

`R2D_GridPoint`, `R2D_GridAStar()`, `R2D_GridFloodFill()` y
`R2D_GridLineOfSight()` cubren pathfinding simple sobre grids rectangulares. Tambien hay
helpers de distancia Manhattan/euclidea. Para mapas Tiled, los wrappers
`R2D_TilemapFindPath()`, `R2D_TilemapFloodFill()` y `R2D_TilemapLineOfSight()` usan una
capa de tiles como bloqueo: cualquier tile no cero se considera solido.

`r2d_collect` usa estos helpers en modo debug: con `F3`, dibuja una ruta A* desde el
jugador hasta `FountainTrigger` y colorea la linea segun haya vision directa o no.

## Cinematicas y eventos

`R2D_Cinematic` ejecuta secuencias pequenas de pasos manuales. Soporta esperar, mover la
camara hacia un punto, bloquear input, mostrar dialogo, activar/desactivar bits de flags,
lanzar SFX y reproducir musica. El juego llama a `R2D_CinematicUpdate()` cada frame y puede
usar `R2D_CinematicInputLocked()` para congelar control del jugador mientras la secuencia
esta activa.

`r2d_collect` lo usa en `FountainTrigger`: al pisar el trigger por primera vez lanza un
SFX, mueve la camara a la fuente, muestra un dialogo y marca un flag para no repetir el
evento.

## Debug In-Game

`R2D_DebugInfoDefault()` y `R2D_DebugDrawOverlay()` dibujan un panel compacto para
depuracion en runtime: FPS, frame time, entidades, assets montados, memoria estimada y
tile bajo cursor. `r2d_collect` lo activa con `F3`, junto al draw de colisiones, objetos,
triggers, camara y ruta A*.

## Entidades

`R2D_EntityWorld` es un pool fijo de entidades ligeras, pensado para juegos pequenos y
predecibles. Cada entidad tiene ID estable, `position`, `velocity`, `bounds`, `type`,
`layer`, `flags`, `user_data` y callbacks opcionales de update/draw. `R2D_EntitySpawn()`
crea entidades, `R2D_EntityDestroy()` invalida su ID anterior de forma segura, y
`R2D_EntityFindByType()` / `R2D_EntityFindByLayer()` permiten recorrer grupos concretos.

Si una entidad no define callback de update, `R2D_EntityWorldUpdate()` aplica su velocidad
a la posicion y mantiene `bounds.x/y` sincronizados. `r2d_collect` usa este sistema para
las monedas recogibles.

## Animaciones por nombre

`R2D_AnimSet` permite registrar clips con nombre y reproducirlos sin guardar `R2D_Anim`
sueltas en cada juego. Se puede llenar por codigo con `R2D_AnimSetAdd()` o cargar desde
un archivo `.r2anim`:

```ini
idle=0,1,1,true
walk=0,4,8,true
attack=4,2,10,false
hurt=0,1,1,false
```

El formato es `nombre=primer_frame,cantidad,fps,loop`. Luego se reproduce con
`R2D_AnimPlayNamed()`. `r2d_collect` carga `assets/animations/collect_player.r2anim` y
`assets/animations/coin.r2anim`, con fallback manual si faltan.

## Atlas y metadata de sprites

`R2D_SpriteAtlas` carga un `.r2atlas` editable para asociar nombres, pivots, hitboxes y
hurtboxes a frames de una textura. El formato actual cubre spritesheets en grid y esta
pensado para que un importador externo pueda generar el mismo texto:

```ini
texture=textures/DawnLike/Commissions/Mage.png
frame_width=16
frame_height=16
frame=south_0,0,8,8,3,2,10,13,2,1,12,14
```

La linea `frame` usa `nombre,index,pivot_x,pivot_y,hit_x,hit_y,hit_w,hit_h,hurt_x,hurt_y,hurt_w,hurt_h`.
Las cajas son opcionales si el frame solo necesita nombre y pivot. `R2D_DrawAtlasFrame()`
dibuja por nombre, y `R2D_SpriteAtlasHitbox()` / `R2D_SpriteAtlasHurtbox()` devuelven las
cajas ya colocadas en mundo o pantalla. `r2d_collect` usa
`assets/atlases/collect_player.r2atlas` para dibujar el jugador y mostrar cajas en `F3`.

## Particulas

`R2D_ParticleSystem` es un pool fijo de particulas ligeras. Cada particula guarda
posicion, velocidad, aceleracion, vida, tamano inicial/final, color inicial/final y forma
pixel o circulo. `R2D_ParticleEmitter` permite emitir de forma continua con `emit_rate` o
lanzar bursts manuales con `R2D_ParticleBurst()`.

La primera version incluye presets para `dust`, `hit`, `spark`, `smoke`, `coin` y `star`.
`r2d_particle_example` muestra un emisor siguiendo el raton: click o `Space` lanza bursts,
y `Left/Right` cambia de preset.

## Paletas

`R2D_Palette` guarda una lista pequena de colores para juegos pixel-art. Puede crearse
desde `Color` o desde hex RGBA con `R2D_PaletteFromHex()`, buscar el color mas cercano y
mezclar colores para flashes o fades. `R2D_ImageRecolorPalette()` crea una copia de una
imagen remapeando cada pixel desde una paleta origen a una paleta destino, y
`R2D_LoadTextureFromPalette()` carga directamente una textura recoloreada.

`r2d_palette_example` recorta un item del atlas DawnLike `Items/Money.png`, usa la paleta
DawnBringer 16 como origen y muestra cambios de paleta, flash y fade sobre un asset real.

## Timers y efectos temporales

`R2D_TimerSystem` gestiona timers `after` y `every` con callbacks opcionales. Los timers
usan un pool fijo y devuelven indices cancelables. `R2D_TweenSystem` interpola `float`
con easing lineal, entrada, salida y entrada/salida, pensado para animar valores simples
sin crear objetos pesados.

Encima de esa base hay helpers pequenos para efectos habituales: `R2D_Shake` calcula un
offset de camara temporal, y `R2D_TimeEffects` centraliza hitstop, slow motion, flash y
fade. `R2D_TimeEffectsUpdate()` devuelve un `dt` escalado para la logica de juego: `0`
durante hitstop, reducido durante slow motion, y normal el resto del tiempo.

`r2d_time_example` muestra el sistema con un bloque animado por tweens, un timer repetido,
un `after`, shake de pantalla y overlays de flash/fade.

## Save data y configuracion

`R2D_UserDataPath()` devuelve una ruta de escritura para datos del usuario. En Windows usa
`APPDATA`; en Linux/macOS intenta `XDG_DATA_HOME` o `~/.local/share`. Si no hay una ruta
de usuario disponible, cae junto al ejecutable. El helper crea la carpeta de la aplicacion
si falta.

`R2D_SaveData` cubre la configuracion inicial comun: version, escala de ventana,
fullscreen, volumen master/musica/SFX, progreso y high score. `R2D_SaveDataLoad()` y
`R2D_SaveDataSave()` usan texto plano `clave=valor`, con defaults seguros si el archivo no
existe o faltan claves. El formato incluye `version` para migraciones futuras.

`r2d_save_example` permite modificar progreso, puntuacion, volumen y flags, guardar,
recargar y restaurar defaults.

## Configuracion de runtime

`R2D_RuntimeConfig` carga opciones de arranque desde un archivo `r2d.ini` junto al
ejecutable y permite sobrescribirlas con argumentos de linea de comandos. El `.ini` vive
fuera del paquete de assets para que sea facil editarlo despues de distribuir el juego. El
formato es texto `clave=valor`:

```ini
resolution=320x200
window_scale=4
fullscreen=false
crt=true
master_volume=0.75
music_volume=1.0
sfx_volume=1.0
ui_volume=1.0
ambient_volume=1.0
asset_pack=r2d_collect.assets
```

Flags soportados: `--windowed`, `--fullscreen`, `--scale 3`, `--resolution 400x240`,
`--width 400`, `--height 240`, `--no-crt`, `--volume 0.8`, `--music-volume 0.6`,
`--sfx-volume 1.0` y `--asset-pack path/to/game.assets`. CMake copia el `r2d.ini` del repo
junto a cada ejecutable. `r2d_collect` lo carga desde `GetApplicationDirectory()`, aplica
esos flags antes de `R2D_Init()` y despues aplica volumen/CRT con
`R2D_RuntimeConfigApplyAudio()` y `R2D_RuntimeConfigApplyCrt()`.

`asset_pack` es opcional en el `.ini`: si no se define, el framework intenta montar
automaticamente un `.assets` con el mismo nombre que el ejecutable.

## Localizacion

`R2D_Localization` carga diccionarios editables `clave=texto` desde la carpeta `locale`
junto al ejecutable. Si no encuentra un archivo externo, intenta cargar la misma ruta como
asset, pero el flujo recomendado para juegos distribuidos es dejar `locale/*.r2loc` fuera
del `.assets` para que se pueda modificar sin recompilar ni reempaquetar.

```ini
ui.title=UI example
ui.restart=Restart text
ui.body=Nine-slice windows and typewriter text are useful for RPG dialogs.
```

Se carga con `R2D_LocalizationLoad(&locale, "en")` y se consulta con
`R2D_LocalizationGet(&locale, "ui.title", "UI example")`. El ejemplo `r2d_ui_example`
copia `locale/en.r2loc` y `locale/es.r2loc` junto al exe; pulsa `L` para alternar idioma.
Las secuencias `\n`, `\t` y `\\` se expanden al leer el archivo.

## Asset Cache

`R2D_AssetCache` es una cache opt-in para recursos pesados. Los loaders manuales
(`R2D_LoadTexture()`, `R2D_LoadFragmentShader()`) siguen existiendo y mantienen ownership
manual; si se usa la cache, `R2D_AssetCacheLoadTexture()` y
`R2D_AssetCacheLoadFragmentShader()` devuelven el recurso ya cargado cuando la ruta se
repite. `R2D_AssetCacheReleaseGroup()` libera un grupo concreto, y
`R2D_AssetCacheClear()` libera todo lo que pertenezca a esa cache.

`r2d_ui_example` usa la cache para la textura de UI, como ejemplo pequeno de escena que
carga una textura una vez y la libera al cerrar.

## Hot reload

`R2D_FileWatch` detecta cambios en archivos de desarrollo usando la fecha de modificacion.
Sirve como base general para recargar tilemaps, texturas, presets `.r2sfx`, canciones
`.r2song`, paletas, configs o cualquier archivo propio del juego:

```c
R2D_FileWatch watch;
R2D_FileWatchInit(&watch);
R2D_FileWatchSet(&watch, "tilemaps/collect.json");

if (R2D_FileWatchCheck(&watch)) {
    R2D_TilemapUnload(&map);
    R2D_TilemapLoadTiledJson(&map, "tilemaps/collect.json");
}
```

En `Debug`, `R2D_FileWatchSet()` resuelve con `R2D_AssetPath()`, asi que apunta a la
carpeta `assets` del proyecto. En distribuciones empaquetadas los assets pueden vivir
dentro de `.assets`, donde no hay un archivo individual que vigilar; para mods y configs
editables conviene dejarlos junto al ejecutable, igual que `r2d.ini` y `locale`.

## Log

El log del framework expone `R2D_LogInfo()`, `R2D_LogWarn()` y `R2D_LogError()` por
subsistema. `R2D_LogSetLevel()` filtra por nivel, `R2D_LogOpenFile()` duplica la salida a
un archivo, y `R2D_LogLastError()` conserva el ultimo error registrado por subsistema:

```c
R2D_LogSetLevel(R2D_LOG_LEVEL_INFO);
R2D_LogOpenFile(R2D_UserDataPath("MyGame", "game.log"));
R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "loaded room %d", room_index);
R2D_LogError(R2D_LOG_SUBSYSTEM_ASSETS, "missing texture: %s", path);
```

La salida de consola usa `TraceLog()` de raylib para integrarse con el flujo existente, y
el archivo usa texto con fecha, nivel y subsistema.

## Patrones de juego

Para un top-down, define acciones `left/right/up/down`, convierte dos ejes en un vector
de movimiento, mueve un AABB con `R2D_MoveAndSlide()` o `R2D_TilemapMoveAndSlide()`, y
haz que la camara siga al centro del jugador con `R2D_CameraFollow()`. Usa una capa
`Collision` en Tiled para paredes invisibles y capas `Foreground`/`Above` para dibujar
por encima del jugador.

Para pickups, representa cada moneda o item como una entidad. Al cargar un mapa, crea
entidades desde objetos `type=coin` o desde una capa `Pickups`; en update consulta la
caja del jugador contra las cajas de los pickups, reproduce un SFX y destruye la entidad.
`r2d_collect` es el patron completo: objetos de Tiled, fallback si faltan monedas, SFX,
contador y condicion `ALL CLEAR`.

Para menus, usa `R2D_InputMap` con acciones de navegacion, `R2D_UiNav` para foco y submit,
y dibuja cada opcion con `R2D_DrawUiMenuItem()`, `R2D_DrawUiToggle()` o
`R2D_DrawUiSelector()`. Para dialogos o cajas de RPG, combina `R2D_NineSlice`,
`R2D_Typewriter` y `R2D_LocalizationGet()`.

Para Tiled, mantén nombres convencionales: `PlayerStart`, `Collision`, `Pickups`,
`Foreground`, `Above` y objetos `type=trigger` para eventos. Las propiedades custom son
texto/int/float/bool/color y se leen con `R2D_Tilemap*Property*()`. Los tiles animados,
offset, opacidad, parallax, multiples tilesets y triggers ya estan cubiertos por el loader.

Para empaquetar, compila en `Release` y usa `r2d_pack_game_<target>`. El resultado deja el
ejecutable, `.assets`, `r2d.ini`, `locale` y atribuciones en `build/dist/Release/<target>/`.
Los archivos que quieras editar despues de distribuir el juego deben vivir fuera de
`.assets`.

Quedan fuera a proposito, de momento: mapas infinitos, chunks, base64, compresion e
isometrico/hexagonal. La idea es usar Tiled como editor potente sin convertir el
framework en un motor enorme.

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
`r2d_audio_example.assets`, `r2d_hello_index.assets`, etc. El framework monta
automaticamente el `.assets` con el mismo nombre que el `.exe` y, si no existe, cae al
formato clasico `assets` junto al ejecutable.

El empaquetado se controla con `R2D_PACKAGE_ASSETS` y esta activado por defecto para los
ejemplos. Las herramientas de desarrollo siguen copiando la carpeta `assets`, porque
necesitan listar directorios y guardar archivos editables.

Los targets `r2d_pack_game_<target>` crean una carpeta final en
`build/dist/<config>/<target>/` con el ejecutable, el paquete `.assets` si existe, `r2d.ini`
editable junto al exe, carpeta `locale` editable y `ATTRIBUTION.md`. Si el proyecto
define `LICENSE`, `LICENSE.md`, `LICENSE.txt` o `assets/icon.ico`, tambien se copian. El
target agregado `r2d_pack_game` empaqueta todas las demos registradas.

## Estructura

```text
assets                  Recursos que se copian junto al ejecutable
build/dist              Carpetas finales generadas por r2d_pack_game
locale                  Textos localizados .r2loc editables junto al ejecutable
*.assets                Paquete de assets por ejecutable en Release
assets/audio/sfx        Presets de sintetizador .r2sfx
assets/audio/music      Canciones MIDI
assets/audio/soundfonts Bancos SoundFont .sf2
assets/animations       Clips de animacion .r2anim
assets/atlases          Metadata de sprites .r2atlas
external/tinysoundfont  TinySoundFont y TinyMidiLoader
include/r2d/r2d.h       API publica
src/r2d.c               Implementacion del framework
src/r2d_asset_cache.c   Cache opt-in de texturas y shaders por grupo
src/r2d_assets.c        Carga desde carpeta o paquete .assets
src/r2d_camera.c        Camara 2D simple para coordenadas de mundo y pantalla
src/r2d_cinematic.c     Secuencias de eventos, dialogo, camara, flags y audio
src/r2d_collision.c     AABB, filtros, triggers y move_and_slide
src/r2d_crt.c           Postproceso CRT opcional
src/r2d_debug.c         Overlay de debug in-game y formato de memoria
src/r2d_entity.c        Pool fijo de entidades ligeras con IDs estables
src/r2d_grid.c          A*, flood fill, line of sight y distancias de grid
src/r2d_hot_reload.c    Watchers de archivo para hot reload en desarrollo
src/r2d_locale.c        Diccionarios de localizacion .r2loc
src/r2d_log.c           Log por nivel, archivo y ultimo error por subsistema
src/r2d_audio.c         Sintetizador simple para efectos retro
src/r2d_music.c         Reproduccion MIDI + SoundFont
src/r2d_palette.c       Paletas pequenas, recolor de imagenes, flashes y fades
src/r2d_particle.c      Pool fijo de particulas y emisores retro
src/r2d_runtime.c       Configuracion runtime desde r2d.ini y argumentos
src/r2d_save.c          Save data, config y rutas de usuario
src/r2d_sprite.c        Spritesheets en grid y animacion simple
src/r2d_time.c          Timers, tweens, shake y efectos temporales
src/r2d_tilemap.c       Carga y dibujado basico de mapas Tiled JSON
examples/hello_index    Hello / index del framework
examples/input          Ejemplo de acciones de entrada
examples/ui             Ejemplo de UI, texto y CRT
examples/audio          Ejemplo de SFX y musica
examples/state          Ejemplo de maquina de estados
examples/collision      Ejemplo visual de colisiones, triggers y sensores
examples/particles      Ejemplo visual de particulas y presets
examples/palette        Ejemplo visual de paletas y recolor
examples/time           Ejemplo visual de timers, tweens y efectos temporales
examples/save           Ejemplo visual de save data y configuracion
examples/template       Template minimo de juego
examples/platformer     Ejemplo de patron platformer
examples/topdown        Ejemplo de patron top-down
examples/collect        Mini demo jugable de recoger monedas
tools/sfx_editor        Editor sencillo de presets de sonido
tools/midi_player       Reproductor para probar MIDIs con SoundFonts
tools/build_gui.ps1     Launcher visual simple para builds
tools/build_targets.json Targets visibles en el launcher
```
