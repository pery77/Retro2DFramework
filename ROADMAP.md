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
- [x] Shader CRT recargable en caliente desde el sandbox.
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
- [x] Demo con spritesheets reales de personaje y monedas.

### Tilemaps

- [x] Carga inicial de mapas Tiled JSON ortogonales y finitos.
- [x] Soporte de capas `tilelayer`.
- [x] Soporte de capas `objectgroup`.
- [x] Soporte de tilesets incrustados con imagen unica.
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
- [x] Sandbox general de pruebas vivas.
- [x] Demo jugable `collect`.
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

- [ ] Sistema de estados o escenas.
  - Estados tipo `Title`, `Game`, `Pause`, `GameOver`, `Options`.
  - Callbacks `enter`, `update`, `draw`, `exit`.
  - Cambio de estado y stack opcional para overlays como pausa.

- [ ] UI retro basica.
  - Menus, botones, selectores, sliders y toggles.
  - HUDs, barras de vida, contadores e iconos.
  - Ventanas 9-slice y cajas de dialogo.
  - Navegacion por teclado/gamepad.

- [ ] Texto y fuentes bitmap.
  - Carga de fuentes pixel.
  - Medicion de texto, alineacion y wrapping.
  - Sombra/outline/tint.
  - Texto progresivo tipo RPG.
  - Control de espaciado entre letras y lineas.

- [ ] Colision 2D mas completa.
  - AABB generico.
  - Triggers y sensores.
  - Capas y mascaras de colision.
  - Resolucion de movimiento.
  - Helper tipo `move_and_slide`.

- [ ] Sistema de entidades ligero.
  - IDs estables.
  - Pools fijos o arrays gestionados.
  - Posicion, tipo, flags y callbacks.
  - Busqueda por tipo/capa.
  - Base reutilizable para enemigos, pickups, proyectiles y puertas.

- [ ] Asset manager con cache.
  - Cargar texturas, sonidos, musica y shaders una sola vez.
  - Handles o IDs para assets.
  - Liberacion por grupo, escena o global.
  - Evitar duplicados y leaks.

- [ ] Particulas retro.
  - Emisores simples.
  - Vida, velocidad, gravedad, color y tamano.
  - Presets para polvo, impacto, chispas, humo, monedas y estrellas.

- [ ] Timers, tweens y efectos temporales.
  - `after`, `every` y timers manuales.
  - Interpolaciones.
  - Camera shake.
  - Hitstop, slow motion, flashes y fades.

- [ ] Save data y configuracion.
  - Guardado de opciones.
  - Guardado de progreso y high scores.
  - Paths de usuario por plataforma.
  - Formato simple versionable.

### Prioridad media

- [ ] Sistema de paletas.
  - Paletas globales y por sprite.
  - Recolor, flashes y fades.
  - Cambios de paleta por shader si encaja.

- [ ] Tilemaps mas ricos.
  - Propiedades custom de Tiled.
  - Propiedades de objetos y capas.
  - Tiles animados.
  - Capas parallax.
  - Triggers desde object layers.
  - Multiples tilesets mas robustos.

- [ ] Pathfinding y helpers de grid.
  - A* sobre tilemap.
  - Flood fill.
  - Line of sight.
  - Distancias Manhattan y euclidea.

- [ ] Mixer de audio mas comodo.
  - Grupos `music`, `sfx`, `ui`, `ambient`.
  - Volumen por grupo.
  - Fade in/out.
  - Crossfade de musica.
  - Pitch random para SFX repetidos.

- [ ] Herramientas de debug in-game.
  - Overlay de FPS, memoria y assets.
  - Inspector simple de entidades.
  - Tile bajo cursor.
  - Draw de camara, colisiones y triggers.
  - Consola o comandos debug.

- [ ] Configuracion de runtime.
  - Archivo `r2d.ini` o equivalente.
  - Resolucion, escala, fullscreen, volumen y CRT.
  - Flags de arranque como `--windowed`, `--scale`, `--asset-pack`.

- [ ] Cinematicas y eventos.
  - Secuencias simples.
  - Esperar, mover camara, bloquear input, mostrar dialogo.
  - Activar flags y lanzar SFX/musica.

- [ ] Animaciones por nombre.
  - Clips como `idle`, `walk`, `attack`, `hurt`.
  - Registro manual y carga desde archivo `.r2anim`.

- [ ] Atlas y metadata de sprites.
  - Frames nombrados.
  - Pivots.
  - Hitboxes y hurtboxes.
  - Posible import desde herramientas externas.

- [ ] Packaging de juego final.
  - Empaquetar ejecutable, assets, config, icono y licencias.
  - Perfil Debug/Release para distribucion.
  - Comando o herramienta tipo `r2d_pack_game`.

### Prioridad baja

- [ ] Hot reload general.
  - Tilemaps.
  - Texturas.
  - SFX.
  - Canciones `.r2song`.
  - Paletas y configs.

- [ ] Sistema de log.
  - `R2D_LogInfo`, `R2D_LogWarn`, `R2D_LogError`.
  - Niveles configurables.
  - Log a consola y archivo.
  - Ultimo error por subsistema.

- [ ] Mas ejemplos de referencia.
  - Template de juego limpio.
  - Ejemplo input.
  - Ejemplo UI/dialogo.
  - Ejemplo platformer.
  - Ejemplo top-down.
  - Ejemplo particulas.
  - Ejemplo save data.

- [ ] Documentacion de patrones.
  - Como crear un top-down.
  - Como crear pickups.
  - Como crear menus.
  - Como usar Tiled con convenciones del framework.
  - Como empaquetar un juego.

## Orden sugerido de implementacion

1. Estados/escenas.
2. Texto bitmap y UI retro basica.
3. Asset manager con cache.
4. Entidades y colision 2D mas completa.
5. Timers, tweens, shake y particulas.
6. Save data y configuracion.
7. Ampliacion de Tiled con propiedades custom y tiles animados.
8. Mixer de audio por grupos.
9. Packaging final de juegos.

## Notas de direccion

- Mantener el framework pequeno, C-friendly y facil de leer.
- Preferir sistemas simples y composables antes que un motor grande.
- Usar los ejemplos como pruebas vivas y documentacion practica.
- Conservar la estetica retro como decision central: pixel-perfect, paletas, fuentes bitmap,
  audio chiptune y herramientas rapidas.
- Evitar dependencias pesadas salvo que resuelvan un problema claro.
- 2026-05-12: Input inicial implementado con `R2D_InputMap`, acciones nombradas,
  bindings de teclado/raton/gamepad, sticks con deadzone y consultas `down/pressed/released/value`.
  `sandbox` y `collect` ya lo usan para movimiento y acciones basicas.
