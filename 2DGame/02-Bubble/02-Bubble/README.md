# Bugs Bunny Crazy Castle 3 — 2D Clone

A faithful 2D clone of **Bugs Bunny Crazy Castle 3** built from scratch in C++ with OpenGL. Developed as a university project for the *Videojocs 2D* course.

**Authors:** Carolina Rodríguez Ujano & Mateus Grandolfi Albuquerque

---

## Gameplay

Collect all three keys on each level, then reach the exit door to advance. Five levels with increasing difficulty, a hidden secret room, and four usable items to help you deal with enemies.

A gameplay demo is included: `demo.avi`

---

## Features

- **5 levels** with tile-based maps loaded from text files
- **Player movement:** walk left/right, climb ladders, use jump floors and warp floors, enter doors
- **4 enemies per level** with distinct AI behaviours:
  - *Tweety (Piolin)* — horizontal patrol
  - *Sylvester (Lucas)* — horizontal chase
  - *Ghost* — horizontal patrol
  - *Tasmanian Devil (Tasmania)* — aggressive chase, follows the player anywhere and transforms
- **3 lives** with respawn; losing all lives returns to the main menu
- **4 usable items:**
  - **100t Weight** — push it to crush enemies on contact or by falling
  - **Bomb** — explodes and eliminates nearby enemies
  - **Boots** — temporarily increase player speed
  - **Clock** — freezes all enemies for a duration
- **Secret room** accessible via hidden doors, containing bonus loot and a chest
- **Full HUD:** life hearts, collected keys, carried item icon, god-mode indicator
- **4 game screens:** main menu, gameplay, instructions, credits
- **Audio:** per-level background music, sound effects for every action, mute toggle
- **Debug keys:** god mode, instant key collection, direct level select

---

## Controls

| Key | Action |
|-----|--------|
| Arrow Left / Right | Move |
| Arrow Up / Down | Climb/descend ladders |
| Arrow Up | Open doors, use warp floors, open chest |
| Z | Use carried item |
| P | Pause |
| ESC | Return to main menu |
| M | Mute / unmute audio |
| G | Toggle god mode (debug) |
| K | Collect all keys (debug) |
| 1 – 5 | Jump to level N (debug) |

---

## Building

### Dependencies

- **OpenGL / GLEW / GLFW3** — rendering
- **GLM** — math
- **SOIL** (Simple OpenGL Image Library) — texture loading (source included under `../../../libs/`)
- **SDL2 + SDL2\_mixer** *(optional)* — audio; sound is silently disabled if not found

On Fedora/RHEL:
```
sudo dnf install glew-devel glfw-devel mesa-libGL-devel glm-devel SDL2-devel SDL2_mixer-devel
```

On Ubuntu/Debian:
```
sudo apt install libglew-dev libglfw3-dev libglm-dev libsdl2-dev libsdl2-mixer-dev
```

### Compile

```bash
make
```

Produces the `bubble` executable in the project directory.

```bash
make clean   # remove object files and binary
```

---

## Project Structure

```
02-Bubble/
├── main.cpp            Entry point
├── Game.{h,cpp}        Singleton game loop, state machine, UI rendering
├── Scene.{h,cpp}       Level management, entities, collision, HUD
├── Player.{h,cpp}      Player controller and physics
├── Enemy.{h,cpp}       Enemy AI (patrol, chase, transform)
├── TileMap.{h,cpp}     Tile map loading and rendering
├── Sprite.{h,cpp}      Sprite/animation system
├── Texture.{h,cpp}     OpenGL texture wrapper
├── Shader.{h,cpp}      GLSL shader compilation
├── ShaderProgram.{h,cpp} Shader program linking
├── AnimKeyframes.h     Animation keyframe definitions
├── levels/             Level tile maps (level_1.txt … level_5.txt)
├── secrets/            Secret room tile maps (secret1.txt … secret5.txt)
├── images/             Sprite sheets, tilesets, UI assets
├── shaders/            GLSL vertex and fragment shaders
└── sound/              Music (MP3) and sound effects (WAV)
```

---

## Technical Notes

- Window resolution: **640 × 480**
- Renderer: OpenGL with custom GLSL shaders (`shaders/texture.vert/frag`)
- Tile types (blocks, cliffs, ladders, doors, jump floors, warp floors) are classified per-level in `Scene::initMap`
- Camera follows the player with zoom support
- Audio is conditionally compiled; the game runs without SDL2 (no sound)
