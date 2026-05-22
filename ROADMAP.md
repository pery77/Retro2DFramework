# Retro2DFramework Roadmap

Este archivo sirve como contexto rapido del estado del framework: que piezas ya existen,
que falta para hacerlo mas profesional, y en que orden conviene avanzar. La idea es que
cualquier chat o sesion futura pueda entender el proyecto de un vistazo.

## Estado actual

### Core y ventana

- [x] Framework C pequeno sobre raylib.
- [x] Configuracion basica con resolucion virtual, escala de ventana y titulo.
- [x] Renderizado a textura para mantener pixel-perfect.
- [x] Bucle principal basado en callbacks `init`, `update`, `draw` y `shutdown`.
- [x] Helpers para tamano virtual, coordenadas virtuales del raton y rectangulos.
- [x] Fullscreen toggle.
- [x] Captura de screenshots.

### Render retro

- [x] Postproceso CRT opcional.
- [x] Shader CRT recargable en caliente desde los ejemplos.
- [x] Textura de ruido para el shader CRT.
- [x] Helper `R2D_ColorFromHex`.

### Camara

- [x] Camara 2D propia con posicion y viewport.
- [x] Seguimiento de objetivo.
- [x] Clamp a rectangulo de mapa.
- [x] Conversion world/screen.
- [x] Calculo de viewport visible.

### Sprites y animacion

- [x] Carga de spritesheets.
- [x] Creacion de spritesheet desde textura ya existente.
- [x] Animaciones por rango de frames.
- [x] `R2D_AnimPlayer` con update, play, stop y frame actual.
- [x] Dibujado de sprites, frames y animaciones.
- [x] Flip horizontal.
- [x] Demo con spritesheet procedural.
- [x] Demo con spritesheets reales migrada al set grafico comun definitivo.
- [x] Demo con spritesheets procedurales para evitar dependencias temporales de arte.

### Tilemaps

- [x] Carga inicial de mapas Tiled JSON ortogonales y finitos.
- [x] Soporte de capas `tilelayer`.
- [x] Soporte de capas `objectgroup`.
- [x] Soporte de multiples tilesets incrustados con imagen unica.
- [x] Soporte de tilesets externos `.tsx` simples.
- [x] Busqueda de capas por nombre.
- [x] Busqueda de objetos por nombre y tipo.
- [x] Colision por capa de tiles.
- [x] Dibujado completo de tilemap.
- [x] Dibujado de capas concretas.
- [x] Dibujado optimizado del area visible por camara.
- [x] Debug draw de colisiones y objetos.
- [x] Convencion de capa `Collision`.
- [x] Convencion de objeto `PlayerStart`.

### Audio y musica

- [x] Inicializacion/cierre de audio propio.
- [x] Volumen master.
- [x] Sintetizador SFX con varias formas de onda.
- [x] Envolvente, pitch slide, vibrato, arpegio, duty sweep y filtros por voz.
- [x] Presets SFX compilados: coin, jump, laser, hit, explosion y powerup.
- [x] Carga y guardado de `.r2sfx`.
- [x] Reproduccion de SFX y tonos.
- [x] Musica MIDI con SoundFont usando TinySoundFont/TinyMidiLoader.
- [x] Carga de canciones `.r2song`.
- [x] Loop, play, stop, pause y resume de musica.
- [x] Volumen de musica.
- [x] Control por canal MIDI: mute, volumen, banco, programa y actividad.
- [x] Reproductor MIDI con seleccion de MIDI/SoundFont.
- [x] Guardado/carga de configuraciones `.r2song` desde el reproductor.

### Assets y empaquetado

- [x] Helper `R2D_AssetPath`.
- [x] Comprobacion de existencia de assets.
- [x] Carga de datos binarios y texto.
- [x] Carga de texturas y shaders desde rutas de assets.
- [x] Montaje/desmontaje de asset packs.
- [x] Herramienta `r2d_pack`.

### Herramientas y ejemplos

- [x] `build.bat` como punto unico de entrada para CMake.
- [x] Build Debug/Release.
- [x] Seleccion de targets concretos.
- [x] `build_gui.bat` con launcher visual.
- [x] Lista de targets en `tools/build_targets.json`.
- [x] `r2d_hello_index` como hello world/indice de ejemplos Retro2D.
- [x] Ejemplo dedicado de input.
- [x] Ejemplo dedicado de UI/texto.
- [x] Ejemplo dedicado de audio.
- [x] Ejemplo dedicado de estados.
- [x] Ejemplo dedicado de colisiones.
- [x] Ejemplo dedicado de particulas.
- [x] Ejemplo dedicado de timers/tweens.
- [x] Ejemplo dedicado de save data.
- [x] Demo jugable `collect` migrada al set grafico comun definitivo.
- [x] Editor de SFX.
- [x] Reproductor MIDI.

## Roadmap recomendado

### Prioridad alta

- [x] Sistema de input propio.
  - [x] Acciones como `move_left`, `jump`, `attack`, `pause`.
  - [x] Soporte de teclado, gamepad y raton.
  - [x] Estados `down`, `pressed` y `released`.
  - [x] Remapping en memoria y perfiles por jugador mediante mapas separados.
  - [x] Deadzones para sticks.

- [x] Sistema de estados o escenas.
  - [x] Estados tipo `Title`, `Game`, `Pause`, `GameOver`, `Options`.
  - [x] Callbacks `enter`, `update`, `draw`, `exit`.
  - [x] Cambio de estado y stack opcional para overlays como pausa.

- [x] UI retro basica.
  - [x] Menus y selectores.
  - [x] Botones, sliders y toggles dibujados.
  - [x] HUDs, barras, paneles y contadores simples.
  - [x] Ventanas 9-slice.
  - [x] Cajas de dialogo simples.
  - [x] Navegacion por teclado/gamepad para menus simples.

- [x] Texto y fuentes bitmap.
  - [x] Carga de fuentes pixel.
  - [x] Medicion de texto, alineacion y wrapping.
  - [x] Sombra/outline/tint.
  - [x] Texto progresivo tipo RPG.
  - [x] Control de espaciado entre letras y lineas.

- [x] Colision 2D mas completa.
  - [x] AABB generico.
  - [x] Triggers y sensores.
  - [x] Capas y mascaras de colision.
  - [x] Resolucion de movimiento.
  - [x] Helper tipo `move_and_slide`.

- [x] Sistema de entidades ligero.
  - [x] IDs estables.
  - [x] Pools fijos o arrays gestionados.
  - [x] Posicion, tipo, flags y callbacks.
  - [x] Busqueda por tipo/capa.
  - [x] Base reutilizable para enemigos, pickups, proyectiles y puertas.

- [x] Asset manager con cache.
  - [x] Cargar texturas y shaders una sola vez por cache.
  - [x] Handles simples usando los tipos raylib cacheados.
  - [x] Liberacion por grupo, escena o global.
  - [x] Evitar duplicados y leaks en recursos opt-in.

- [x] Particulas retro.
  - [x] Emisores simples.
  - [x] Vida, velocidad, gravedad, color y tamano.
  - [x] Presets para polvo, impacto, chispas, humo, monedas y estrellas.

- [x] Timers, tweens y efectos temporales.
  - [x] `after`, `every` y timers manuales.
  - [x] Interpolaciones.
  - [x] Camera shake.
  - [x] Hitstop, slow motion, flashes y fades.

- [x] Save data y configuracion.
  - [x] Guardado de opciones.
  - [x] Guardado de progreso y high scores.
  - [x] Paths de usuario por plataforma.
  - [x] Formato simple versionable.

### Prioridad media

- [x] Sistema de paletas.
  - [x] Paletas globales y por sprite.
  - [x] Recolor, flashes y fades.

- [x] Tilemaps mas ricos.
  - [x] Propiedades custom de Tiled.
  - [x] Propiedades de objetos y capas.
  - [x] Tiles animados.
  - [x] Capas parallax.
  - [x] Triggers desde object layers.
  - [x] Multiples tilesets mas robustos.

- [x] Pathfinding y helpers de grid.
  - [x] A* sobre tilemap.
  - [x] Flood fill.
  - [x] Line of sight.
  - [x] Distancias Manhattan y euclidea.

- [x] Mixer de audio mas comodo.
  - [x] Grupos `music`, `sfx`, `ui`, `ambient`.
  - [x] Volumen por grupo.
  - [x] Fade in/out.
  - [x] Crossfade de musica.
  - [x] Pitch random para SFX repetidos.

- [ ] Herramientas de debug in-game.
  - [x] Overlay de FPS, memoria y assets.
  - [x] Inspector simple de entidades.
  - [x] Tile bajo cursor.
  - [x] Draw de camara, colisiones y triggers.
  - Consola o comandos debug.

- [x] Configuracion de runtime.
  - [x] Archivo `r2d.ini` o equivalente.
  - [x] Resolucion, escala, fullscreen, volumen y CRT.
  - [x] Flags de arranque como `--windowed`, `--scale`, `--asset-pack`.

- [x] Cinematicas y eventos.
  - [x] Secuencias simples.
  - [x] Esperar, mover camara, bloquear input, mostrar dialogo.
  - [x] Activar flags y lanzar SFX/musica.

- [x] Animaciones por nombre.
  - [x] Clips como `idle`, `walk`, `attack`, `hurt`.
  - [x] Registro manual y carga desde archivo `.r2anim`.

- [x] Atlas y metadata de sprites.
  - [x] Frames nombrados.
  - [x] Pivots.
  - [x] Hitboxes y hurtboxes.
  - [x] Posible import desde herramientas externas.

- [x] Packaging de juego final.
  - [x] Empaquetar ejecutable, assets, config, icono opcional y licencias.
  - [x] Perfil Debug/Release para distribucion.
  - [x] Comando o herramienta tipo `r2d_pack_game`.

- [x] Localización.
  - [x] Sistema para localizar fácil juegos.
  - [x] Archivos `.r2loc` en una carpeta junto al .exe, para que la comunidad pueda hacer traducciones.

### Prioridad baja

- [x] Hot reload general.
  - [x] Tilemaps.
  - [x] Texturas.
  - [x] SFX.
  - [x] Canciones `.r2song`.
  - [x] Paletas y configs.

- [x] Sistema de log.
  - [x] `R2D_LogInfo`, `R2D_LogWarn`, `R2D_LogError`.
  - [x] Niveles configurables.
  - [x] Log a consola y archivo.
  - [x] Ultimo error por subsistema.

- [x] Mas ejemplos de referencia.
  - [x] Template de juego limpio.
  - [x] Ejemplo input.
  - [x] Ejemplo UI/dialogo.
  - [x] Ejemplo audio.
  - [x] Ejemplo estados.
  - [x] Ejemplo colisiones.
  - [x] Ejemplo timers/tweens.
  - [x] Ejemplo platformer.
  - [x] Ejemplo top-down.
  - [x] Ejemplo particulas.
  - [x] Ejemplo save data.

- [x] Documentacion de patrones.
  - [x] Como crear un top-down.
  - [x] Como crear pickups.
  - [x] Como crear menus.
  - [x] Como usar Tiled con convenciones del framework.
  - [x] Como empaquetar un juego.

## Orden sugerido de implementacion

1. Asset manager con cache.
2. Entidades y colision 2D mas completa.
3. Timers, tweens, shake y particulas.
4. Save data y configuracion.
5. Ampliacion de Tiled con propiedades custom y tiles animados.
6. Mixer de audio por grupos.
7. Packaging final de juegos.

## Notas de direccion

- Mantener el framework pequeno, C-friendly y facil de leer.
- Preferir sistemas simples y composables antes que un motor grande.
- Usar los ejemplos como pruebas vivas y documentacion practica.
- Conservar la estetica retro como decision central: pixel-perfect, paletas, fuentes bitmap,
  audio chiptune y herramientas rapidas.
- Evitar dependencias pesadas salvo que resuelvan un problema claro.
- 2026-05-12: Input inicial implementado con `R2D_InputMap`, acciones nombradas,
  bindings de teclado/raton/gamepad, sticks con deadzone y consultas `down/pressed/released/value`.
  `hello_index` y `collect` ya lo usan para movimiento y acciones basicas.
- 2026-05-12: Estados/escenas iniciales implementados con `R2D_StateMachine`,
  callbacks `enter/update/draw/exit`, cambio de estado y stack para overlays.
  el ejemplo principal usaba un estado `Game` y un overlay `Pause`.
- 2026-05-12: el ejemplo principal deja de ser una mini demo jugable y pasa a ser el
  onboarding de Retro2D: pantallas simples para input, audio/musica, sprites/tilemap
  y runtime/estados, con feedback visual y comentarios en ingles en el codigo.
- 2026-05-12: Texto/UI inicial implementado con `R2D_TextStyle`, carga de fuentes
  bitmap, medicion, alineacion, wrapping, sombra/outline y helpers dibujados para
  paneles, botones, toggles, sliders y barras. El ejemplo principal suma pantalla `UI`.
- 2026-05-12: `R2D_LoadBitmapFont` usa `LoadFont` para PNG bitmap/XNA-style;
  el ejemplo principal prueba fuentes de `assets/fonts` con tamanos legibles.
- 2026-05-18: UI ampliada con `R2D_UiNav`, items de menu, selector y dialogo
  simple. El ejemplo principal tiene controles navegables con up/down, left/right y submit.
- 2026-05-18: UI/texto cerrados en primera version con `R2D_NineSlice` para
  ventanas escalables y `R2D_Typewriter` para texto progresivo tipo RPG. El ejemplo principal
  los muestra en la pantalla `UI`.
- 2026-05-18: Nueva direccion de ejemplos: cada sistema debe tener un ejemplo
  dedicado que funcione como documentacion viva. El ejemplo principal vuelve a ser un indice
  pequeno/hello, y se crean `r2d_input_example` y `r2d_ui_example`. Evitar depender
  de graficos temporales; el proyecto usara un set comun de assets libres elegido
  aparte.
- 2026-05-18: Se suman `r2d_audio_example` y `r2d_state_example`, ambos sin
  dependencia de graficos externos para mantenerlos como documentacion clara.
- 2026-05-19: Cambio de direccion de assets por licencias. Los ejemplos base
  deben depender solo de graficos con licencia clara para redistribucion dentro
  del repo. `r2d_input_example`, `r2d_ui_example`, `r2d_audio_example` y
  `r2d_state_example` siguen sanos; `r2d_collect` pasa a usar sprites
  procedurales y un tilemap seguro mientras queda pendiente su migracion visual
  definitiva. Se retiran el mapa anterior de `collect` y
  `tilemaps/Set 1.tsx` porque dependian del tileset retirado. Evitar usar
  `assets/textures/Hero/Full Spritesheet.png` en ejemplos del framework hasta
  confirmar si se puede redistribuir en el repositorio. Se anade
  `assets/ATTRIBUTION.md` como indice corto de creditos/licencias.
- 2026-05-19: `r2d_collect` vuelve a usar graficos reales del set comun:
  `textures/DawnLike/Commissions/Mage.png` para el jugador y `textures/Coin.png`
  para monedas. El mapa usado por la demo es `tilemaps/collect.json`; si el
  mapa no define objetos `Coin*`, la demo crea monedas de fallback para seguir
  siendo jugable.
- 2026-05-19: Tilemaps actualizados para cargar y dibujar multiples tilesets
  en el mismo mapa. Antes `R2D_Tilemap` elegia un unico tileset por `firstgid`,
  asi que mapas de Tiled con walls/floors/props en imagenes distintas solo
  renderizaban una parte.
- 2026-05-19: UI/texto suma `R2D_LoadFont` y `R2D_UnloadFont` como wrappers
  simples sobre raylib `LoadFont`/`UnloadFont`, validos para TTF/OTF/PNG. El
  ejemplo `r2d_ui_example` carga una TTF de DawnLike y la muestra dentro de la
  ventana 9-slice.
- 2026-05-19: Core suma `R2D_RequestClose` para pedir el cierre limpio desde
  juego/estados. `r2d_state_example` anade un estado `Exit`: desde `Pause`,
  `Esc` quita el overlay y `Submit` cambia a `Exit` y cierra la app.
- 2026-05-20: Colision 2D inicial completada con AABB generico, `R2D_Collider`,
  filtros por capa/mascara, triggers, query rectangular y `R2D_MoveAndSlide`.
  Tilemaps suman helpers para convertir tiles solidos visibles en colliders y
  mover AABB contra una capa `Collision`; `r2d_collect` ya usa esa ruta.
- 2026-05-20: La capa de colision queda orientada a complementar raylib, no a
  reemplazarlo: las pruebas geometricas usan `CheckCollision*` de raylib y
  Retro2D aporta filtros, triggers, resultados y resolucion de movimiento.
  Se anaden queries filtradas por punto y circulo para sensores comunes.
- 2026-05-20: Sistema de entidades ligero implementado con `R2D_EntityWorld`,
  pool fijo, IDs estables con generacion, tipo/capa/flags, posicion, velocidad,
  bounds, `user_data`, callbacks opcionales y busqueda por tipo/capa. `r2d_collect`
  migra las monedas a entidades para cubrir pickups reales.
- 2026-05-20: Se anade `r2d_collision_example`: oculta el cursor del sistema con
  `HideCursor()`, mueve un objeto con el raton virtual y muestra solidos, triggers,
  sensores de punto y sensores de circulo usando la capa de colision de Retro2D.
- 2026-05-20: Asset cache inicial implementada con `R2D_AssetCache`, carga cacheada
  de texturas y fragment shaders, liberacion por grupo y `Clear` global de la cache.
  Los loaders manuales se conservan; `r2d_ui_example` usa la cache para su textura UI.
- 2026-05-20: Particulas retro implementadas con `R2D_ParticleSystem`, pool fijo,
  `R2D_ParticleEmitter`, emision continua, bursts y presets `dust`, `hit`, `spark`,
  `smoke`, `coin` y `star`. Se anade `r2d_particle_example` como documentacion viva.
- 2026-05-20: Timers/tweens y efectos temporales implementados con `R2D_TimerSystem`,
  `R2D_TweenSystem`, helpers de easing, `R2D_Shake` y `R2D_TimeEffects` para hitstop,
  slow motion, flash y fade. Se anade `r2d_time_example` como documentacion viva.
- 2026-05-20: Save data y configuracion inicial implementados con `R2D_UserDataPath`,
  `R2D_SaveData`, carga/guardado en texto `clave=valor`, version de formato, opciones,
  progreso y high score. Se anade `r2d_save_example` como documentacion viva.
- 2026-05-20: Prioridad media iniciada con sistema de paletas: `R2D_Palette`,
  creacion desde `Color` o hex RGBA, busqueda de color cercano, mezcla para flash/fade
  y recolor de imagen/textura desde paleta origen a destino. Se anade
  `r2d_palette_example` con un item real de DawnLike, paleta DawnBringer 16 y variantes
  de paleta.
- 2026-05-20: Tilemaps mas ricos arrancan con propiedades custom de Tiled en capas y
  objetos. `R2D_TilemapProperty` soporta `string`, `int`, `float`, `bool` y `color`,
  con helpers de busqueda y conversion segura. `tilemaps/collect.json` define
  `debug_color` en la capa `Collision` y `r2d_collect` lo usa para el overlay F3.
- 2026-05-21: Tilemaps suman triggers desde object layers. Los objetos `type=trigger`,
  `type=sensor` o con propiedad `trigger=true` se convierten en colliders sensor con
  `R2D_TilemapTriggerColliders()`, conservando el `R2D_TilemapObject` original en
  `user_data` para leer propiedades como `event`. `r2d_collect` muestra un mensaje al
  pisar `FountainTrigger`.
- 2026-05-21: Capas Tiled leen `opacity`, `offsetx`, `offsety`, `parallaxx` y `parallaxy`.
  Los draws existentes aplican opacidad y offset, y se anade
  `R2D_TilemapDrawLayerParallax()` para dibujar una capa usando un viewport de camara.
  `r2d_collect` usa esa ruta para que mapas con parallax funcionen sin codigo extra.
- 2026-05-21: Tiles animados de Tiled soportados para tilesets JSON incrustados y `.tsx`
  externos simples. Cada tileset guarda animaciones por tile local con frames/duraciones,
  y el renderer resuelve el frame activo con `GetTime()` al dibujar, sin requerir update.
- 2026-05-21: Multiples tilesets quedan mas robustos con soporte de `margin` y `spacing`
  en tilesets de imagen unica, tanto JSON incrustado como `.tsx` externo. El calculo de
  columnas/filas y el rectangulo fuente del renderer respetan esos valores.
- 2026-05-21: Pathfinding y helpers de grid implementados con `R2D_GridPoint`,
  distancias Manhattan/euclidea, Bresenham line of sight, flood fill y A* 4-direcciones.
  Se anaden wrappers sobre tilemap usando una capa solida como bloqueo:
  `R2D_TilemapFindPath`, `R2D_TilemapFloodFill` y `R2D_TilemapLineOfSight`.
  `r2d_collect` dibuja en F3 una ruta hasta `FountainTrigger` como prueba viva.
- 2026-05-21: Mixer de audio por grupos implementado con `music`, `sfx`, `ui` y `ambient`,
  volumen independiente, fades por grupo con `R2D_AudioMixerUpdate()` y pitch random
  para SFX repetidos. Se anade `R2D_MusicCrossfade` para fundir entre canciones.
  `r2d_audio_example` permite probar el fade de musica con `F`.
- 2026-05-21: Herramientas debug in-game arrancan con `R2D_DebugInfo` y
  `R2D_DebugDrawOverlay()`. `r2d_collect` usa `F3` para mostrar FPS, frame time,
  entidades, assets montados, memoria estimada de gameplay/tilemap, tile bajo cursor,
  GID de colision, camara, colisiones, triggers y ruta A*.
- 2026-05-21: Configuracion de runtime completada con `R2D_RuntimeConfig`: carga
  `r2d.ini` junto al ejecutable, aplica resolucion virtual, escala, fullscreen, CRT,
  volumen master y volumen por grupos, y acepta flags como `--windowed`, `--scale`,
  `--resolution` y `--asset-pack`. `r2d_collect` queda conectado a este flujo y CMake
  copia el `.ini` como archivo suelto, fuera del paquete de assets.
- 2026-05-21: Cinematicas y eventos implementados con `R2D_Cinematic`: pasos manuales
  para esperar, mover camara, bloquear input, mostrar dialogo, activar flags, lanzar SFX
  y reproducir musica. `r2d_collect` usa `FountainTrigger` para probar una secuencia con
  sonido, camara, dialogo y flag de evento.
- 2026-05-22: Animaciones por nombre implementadas con `R2D_AnimSet`, registro manual de
  clips y carga de archivos `.r2anim` en formato `nombre=primer_frame,cantidad,fps,loop`.
  `r2d_collect` carga `collect_player.r2anim` para `idle`, `walk`, `attack`, `hurt` y
  `coin.r2anim` para la moneda.
- 2026-05-22: Atlas y metadata de sprites implementados con `R2D_SpriteAtlas`: carga
  `.r2atlas` con textura, frames nombrados, pivots, hitboxes y hurtboxes. `r2d_collect`
  carga `collect_player.r2atlas`, dibuja el jugador por nombre de frame y muestra
  hitbox/hurtbox en el overlay debug.
- 2026-05-22: Packaging final de juegos implementado con targets CMake
  `r2d_pack_game_<target>` y agregado `r2d_pack_game`. Generan carpetas en
  `build/dist/<config>/<target>/` con ejecutable, `.assets` o carpeta `assets`, `r2d.ini`
  editable fuera del paquete, atribuciones/licencias e icono opcional `assets/icon.ico`.
- 2026-05-22: Localizacion implementada con `R2D_Localization` y archivos `.r2loc`
  `clave=texto`. CMake copia la carpeta `locale` junto al ejecutable y las carpetas
  finales de `r2d_pack_game` la dejan fuera del `.assets`. `r2d_ui_example` alterna
  `en/es` con `L` como prueba viva.
- 2026-05-22: Prioridad baja cerrada. Se anade `R2D_FileWatch` como base general de
  hot reload para archivos editables, `R2D_Log*` con niveles, consola, archivo y ultimo
  error por subsistema, y los ejemplos `r2d_template_game`, `r2d_platformer_example` y
  `r2d_topdown_example`.
- 2026-05-22: README documenta patrones practicos para top-down, pickups, menus,
  convenciones de Tiled y empaquetado final.
