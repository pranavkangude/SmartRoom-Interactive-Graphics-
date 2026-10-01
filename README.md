# SmartRoom: Interactive 2D Room and Furniture Layout Planner

A Computer Graphics course project built with **C++ and OpenGL (FreeGLUT)**.
SmartRoom lets you define a room to scale, place furniture in it, and rearrange everything with the mouse and keyboard, with instant visual feedback when a placement is invalid.

## Features

- Room drawn to scale from user-entered width and length (in feet), with grid, walls, a door with swing arc, and a window
- Six furniture types drawn from OpenGL primitives: bed, sofa, round table, chair, wardrobe, desk
- Add, select, move, rotate, scale and delete furniture
- Snap-to-grid
- Boundary and collision detection (Separating Axis Theorem for rotated items); invalid items turn red and snap back if dropped in an invalid place
- Door-swing clearance zone that furniture cannot block
- Undo / redo (up to 100 steps)
- Save and load layouts to a text file (`layout.txt`)
- Zoom and pan (window-to-viewport mapping)
- On-screen free floor-area percentage

## Controls

| Action | Input |
|---|---|
| Add furniture (bed, sofa, table, chair, wardrobe, desk) | `1` `2` `3` `4` `5` `6` |
| Select / move | Left-click, drag |
| Rotate 90° / 15° | `R` / `E` |
| Scale selected item | `+` / `-` |
| Delete selected item | `X` or `Delete` |
| Clear room | `C` |
| Undo / redo | `U` / `Y` (or `Ctrl+Z` / `Ctrl+Y`) |
| Save / load layout | `S` / `L` |
| Zoom | Mouse wheel, or `Z` / `O` |
| Pan | Right-mouse drag, or arrow keys |
| Reset view | `0` |
| Toggle snap-to-grid | `G` |
| Toggle door clearance zone | `D` |
| Quit | `Esc` |

## Build and run

### Windows (MinGW + FreeGLUT)

1. Download the FreeGLUT MinGW package (for example `freeglut-mingw-3.8.0.zip` from https://www.songho.ca/opengl/gl_freeglut.html) and extract it into the project folder.
2. Copy `libfreeglut.dll` (from the package's `bin` folder) next to `main.cpp`.
3. Compile (PowerShell; the quotes are needed because the folder name contains dots):

```
g++ main.cpp state.cpp transform.cpp collision.cpp gfx.cpp furniture.cpp room.cpp history.cpp fileio.cpp ui.cpp interaction.cpp -o smartroom.exe "-Ifreeglut-mingw-3.8.0/freeglut/include" "-Lfreeglut-mingw-3.8.0/freeglut/lib" -lfreeglut -lopengl32 -lglu32
.\smartroom.exe
```

Or simply run `.\build.bat`.

For a 64-bit compiler, use `freeglut/lib/x64` and the DLL from `bin/x64`.

### Linux

```
sudo apt install build-essential freeglut3-dev
g++ main.cpp state.cpp transform.cpp collision.cpp gfx.cpp furniture.cpp room.cpp history.cpp fileio.cpp ui.cpp interaction.cpp -o smartroom -lGL -lGLU -lglut
./smartroom
```

The program asks for the room width and length in feet (for example `12` and `10`) when it starts.

## Code structure

| File | Module | Responsibility |
|---|---|---|
| `main.cpp` | Application | Window setup, display and reshape callbacks, main loop |
| `common.h`, `state.cpp` | Shared data | `Furniture` and `Vec2` types, global state |
| `transform.h/.cpp` | Transformation | Rotation about a centre, corners, picking, window-to-viewport mapping, zoom and pan |
| `collision.h/.cpp` | Collision | Boundary check, SAT overlap, door-swing clearance, `isValid` |
| `room.h/.cpp` | Room | Floor, grid, walls, door and window |
| `furniture.h/.cpp` | Furniture | Furniture sizes, colours and drawing |
| `interaction.h/.cpp` | Interaction | Mouse, wheel and keyboard callbacks, adding items |
| `history.h/.cpp` | Undo / redo | Snapshot stacks |
| `fileio.h/.cpp` | File | Save and load `layout.txt` |
| `ui.h/.cpp` | UI | On-screen status and help text |
| `gfx.h/.cpp` | Drawing helpers | Rectangles, ellipses, text |

## Computer graphics concepts demonstrated

| Concept | Where it is used |
|---|---|
| Line drawing | Walls, grid, door leaf and swing arc |
| Polygon and ellipse filling | Furniture shapes, floor, round table |
| 2D translation, rotation, scaling | Moving, rotating and resizing furniture |
| Rotation about the object's centre | `rotateAbout()`, `glTranslatef` + `glRotatef` |
| Window-to-viewport mapping | World (feet) to screen (pixels), zoom and pan |
| Collision detection (SAT) | Preventing overlap between rotated furniture |
| Picking | Rotating the mouse point by -theta and testing against the axis-aligned rectangle |
| Event handling | Mouse, wheel, keyboard and special-key callbacks |
| Double buffering | Smooth redraw with `glutSwapBuffers` |

## Project status

- [x] M1-M3: window, scaled room, grid, door, window, furniture drawing
- [x] M4-M6: transformations, mouse and keyboard interaction, snap-to-grid, boundary and collision checks
- [x] M7: undo/redo, save/load, zoom and pan, door clearance zone
- [x] Code split into modules (see Code structure)
- [ ] Furniture palette and wall dimension labels
- [ ] Report, screenshots and demo video

## Limitations and future work

- 2D only, with a fixed set of furniture types
- The round table uses its bounding box for collision
- Possible extensions: 3D view with lighting and textures, custom furniture sizes, rule-based auto-arrange, multi-room plans, export to image or PDF

## Author

Pranav - Computer Graphics course project
