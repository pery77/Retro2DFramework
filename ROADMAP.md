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
- [x] Sandbox como hello world/indice de ejemplos Retro2D.
- [x] Ejemplo dedicado de input.
- [x] Ejemplo dedicado de UI/texto.
- [x] Ejemplo dedicado de audio.
- [x] Ejemplo dedicado de estados.
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
  - [x] Ejemplo input.
  - [x] Ejemplo UI/dialogo.
  - [x] Ejemplo audio.
  - [x] Ejemplo estados.
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
  `sandbox` y `collect` ya lo usan para movimiento y acciones basicas.
- 2026-05-12: Estados/escenas iniciales implementados con `R2D_StateMachine`,
  callbacks `enter/update/draw/exit`, cambio de estado y stack para overlays.
  `sandbox` usa un estado `Game` y un overlay `Pause`.
- 2026-05-12: `sandbox` deja de ser una mini demo jugable y pasa a ser el
  onboarding de Retro2D: pantallas simples para input, audio/musica, sprites/tilemap
  y runtime/estados, con feedback visual y comentarios en ingles en el codigo.
- 2026-05-12: Texto/UI inicial implementado con `R2D_TextStyle`, carga de fuentes
  bitmap, medicion, alineacion, wrapping, sombra/outline y helpers dibujados para
  paneles, botones, toggles, sliders y barras. `sandbox` suma pantalla `UI`.
- 2026-05-12: `R2D_LoadBitmapFont` usa `LoadFont` para PNG bitmap/XNA-style;
  `sandbox` prueba fuentes de `assets/fonts` con tamanos legibles.
- 2026-05-18: UI ampliada con `R2D_UiNav`, items de menu, selector y dialogo
  simple. `sandbox` tiene controles navegables con up/down, left/right y submit.
- 2026-05-18: UI/texto cerrados en primera version con `R2D_NineSlice` para
  ventanas escalables y `R2D_Typewriter` para texto progresivo tipo RPG. `sandbox`
  los muestra en la pantalla `UI`.
- 2026-05-18: Nueva direccion de ejemplos: cada sistema debe tener un ejemplo
  dedicado que funcione como documentacion viva. `sandbox` vuelve a ser un indice
  pequeno/hello, y se crean `r2d_input_example` y `r2d_ui_example`. Evitar depender
  de graficos temporales; el proyecto usara un set comun de assets libres elegido
  aparte.
- 2026-05-18: Se suman `r2d_audio_example` y `r2d_state_example`, ambos sin
  dependencia de graficos externos para mantenerlos como documentacion clara.
- 2026-05-19: Cambio de direccion de assets por licencias. Los ejemplos base
  deben depender solo de graficos con licencia clara para redistribucion dentro
  del repo. `r2d_input_example`, `r2d_ui_example`, `r2d_audio_example` y
  `r2d_state_example` siguen sanos; `r2d_collect` pasa a usar sprites
  procedurales y el tilemap seguro `r2d_sandbox.json` mientras queda pendiente
  su migracion visual definitiva. Se retiran `tilemaps/collect.json` y
  `tilemaps/Set 1.tsx` porque dependian del tileset retirado. Evitar usar
  `assets/textures/Hero/Full Spritesheet.png` en ejemplos del framework hasta
  confirmar si se puede redistribuir en el repositorio. Se anade
  `assets/ATTRIBUTION.md` como indice corto de creditos/licencias.
- 2026-05-19: `r2d_collect` vuelve a usar graficos reales del set comun:
  `textures/DawnLike/Commissions/Mage.png` para el jugador y `textures/Coin.png`
  para monedas. El mapa usado por la demo es `tilemaps/r2d_sandbox.json`; si el
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
