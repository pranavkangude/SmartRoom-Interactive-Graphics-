# SmartRoom: Interactive 2D Room and Furniture Layout Planner

A Computer Graphics course project built with **C++ and OpenGL (FreeGLUT)**.
SmartRoom lets you define a room to scale, place furniture in it, and rearrange everything with the mouse and keyboard, with instant visual feedback when a placement is invalid.

## Features

- Room drawn to scale from user-entered width and length (in feet), with grid, walls, a door with swing arc, and a window
- Six furniture types drawn from OpenGL primitives: bed, sofa, round table, chair, wardrobe, desk
- Clickable furniture palette (with Undo / Redo / Save / Load / Clear buttons) and hover highlighting
- **Own graphics pipeline:** 3x3 homogeneous matrices for all transformations, Liang-Barsky line clipping, Sutherland-Hodgman polygon clipping, DDA and Bresenham line drawing, and scanline polygon fill (OpenGL is used only to plot pixels)
- Wall dimension lines with length labels, and a live info line for the selected item (size, angle, position)
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
| Add furniture (bed, sofa, table, chair, wardrobe, desk) | Click the palette, or keys `1` `2` `3` `4` `5` `6` |
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
| Toggle dimension labels | `M` |
| Line algorithm: OpenGL / DDA / Bresenham | `B` |
| Polygon fill: own scanline / OpenGL | `F` |
| Clip scene to viewport on / off | `K` |
| Quit | `Esc` |

## Build and run

### Windows (MinGW + FreeGLUT)

1. Download the FreeGLUT MinGW package (for example `freeglut-mingw-3.8.0.zip` from https://www.songho.ca/opengl/gl_freeglut.html) and extract it into the project folder.
2. Copy `libfreeglut.dll` (from the package's `bin` folder) next to `main.cpp`.
3. Compile (PowerShell; the quotes are needed because the folder name contains dots):

```
g++ main.cpp state.cpp matrix.cpp clip.cpp raster.cpp gfx.cpp transform.cpp collision.cpp furniture.cpp room.cpp history.cpp fileio.cpp ui.cpp interaction.cpp -o smartroom_v4.exe "-Ifreeglut-mingw-3.8.0/freeglut/include" "-Lfreeglut-mingw-3.8.0/freeglut/lib" -lfreeglut -lopengl32 -lglu32
.\smartroom_v4.exe
```

Or simply run `.\build.bat`.

For a 64-bit compiler, use `freeglut/lib/x64` and the DLL from `bin/x64`.

### Linux

```
sudo apt install build-essential freeglut3-dev
g++ main.cpp state.cpp matrix.cpp clip.cpp raster.cpp gfx.cpp transform.cpp collision.cpp furniture.cpp room.cpp history.cpp fileio.cpp ui.cpp interaction.cpp -o smartroom -lGL -lGLU -lglut
./smartroom
```

The program asks for the room width and length in feet (for example `12` and `10`) when it starts.

## Code structure

| File | Module | Responsibility |
|---|---|---|
| `main.cpp` | Application | Window setup, display and reshape callbacks, main loop |
| `common.h`, `state.cpp` | Shared data | `Furniture` and `Vec2` types, global state |
| `matrix.h/.cpp` | Transformation | 3x3 homogeneous matrices: translate, rotate, scale, multiply, inverse |
| `transform.h/.cpp` | Transformation | Model and view matrices, rotation about a centre, corners, picking, window-to-viewport mapping, zoom and pan |
| `clip.h/.cpp` | Clipping | Liang-Barsky line clipping, Sutherland-Hodgman polygon clipping |
| `raster.h/.cpp` | Rasterization | DDA, Bresenham, scanline polygon fill (compute pixels only) |
| `collision.h/.cpp` | Collision | Boundary check, SAT overlap, door-swing clearance, `isValid` |
| `room.h/.cpp` | Room | Floor, grid, walls, door, window and dimension lines |
| `furniture.h/.cpp` | Furniture | Furniture sizes, colours and drawing |
| `interaction.h/.cpp` | Interaction | Mouse, wheel and keyboard callbacks, adding items |
| `history.h/.cpp` | Undo / redo | Snapshot stacks |
| `fileio.h/.cpp` | File | Save and load `layout.txt` |
| `ui.h/.cpp` | UI | Furniture palette (buttons, hover, hit testing), status and help text |
| `gfx.h/.cpp` | Drawing pipeline | Local coordinates -> own matrix -> clip -> rasterize; rectangles, ellipses, text |

## Computer graphics concepts demonstrated

| Concept | Where it is used |
|---|---|
| Line drawing (DDA, Bresenham) | Grid, walls, door leaf and swing arc, outlines (`raster.cpp`) |
| Scanline polygon filling | Furniture shapes, floor, round table, door zone (`raster.cpp`) |
| 2D translation, rotation, scaling with 3x3 homogeneous matrices | Moving, rotating and resizing furniture; composite transform `T * R * T^-1` (`matrix.cpp`, `transform.cpp`) |
| Rotation about the object's centre | `rotateAbout()` = translate, rotate, translate back |
| Clipping | Liang-Barsky for lines, Sutherland-Hodgman for polygons, against the canvas viewport (`clip.cpp`) |
| Window-to-viewport mapping | World (feet) to screen (pixels), zoom and pan |
| Collision detection (SAT) | Preventing overlap between rotated furniture |
| Picking | Inverse model matrix maps the mouse point into the furniture's local space, then an axis-aligned rectangle test |
| Event handling | Mouse, wheel, keyboard and special-key callbacks |
| Double buffering | Smooth redraw with `glutSwapBuffers` |

## Project status

- [x] M1-M3: window, scaled room, grid, door, window, furniture drawing
- [x] M4-M6: transformations, mouse and keyboard interaction, snap-to-grid, boundary and collision checks
- [x] M7: undo/redo, save/load, zoom and pan, door clearance zone
- [x] Code split into modules (see Code structure)
- [x] Furniture palette and wall dimension labels
- [x] Own matrices, clipping and rasterization algorithms (stage 1 of the upgrades)
- [ ] 3D preview with lighting
- [ ] Rule-based auto-arrange
- [ ] Report, screenshots and demo video

## Algorithm unit tests

The matrix, clipping and rasterization code does not depend on OpenGL, so it has its own tests:

```
g++ tests/test_algorithms.cpp matrix.cpp clip.cpp raster.cpp -I. -o test_algorithms_v1.exe
.\test_algorithms_v1.exe
```

## Limitations and future work

- 2D only, with a fixed set of furniture types
- The round table uses its bounding box for collision
- Possible extensions: 3D view with lighting and textures, custom furniture sizes, rule-based auto-arrange, multi-room plans, export to image or PDF

## Author

Pranav - Computer Graphics course project
