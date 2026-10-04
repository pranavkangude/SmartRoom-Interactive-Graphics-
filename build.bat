@echo off
REM Build SmartRoom (MinGW + FreeGLUT in .\freeglut-mingw-3.8.0)
g++ main.cpp state.cpp roomshape.cpp matrix.cpp clip.cpp raster.cpp gfx.cpp transform.cpp collision.cpp furniture.cpp room.cpp history.cpp fileio.cpp ui.cpp interaction.cpp view3d.cpp autoarrange.cpp -o smartroom_v7.exe "-Ifreeglut-mingw-3.8.0/freeglut/include" "-Lfreeglut-mingw-3.8.0/freeglut/lib" -lfreeglut -lopengl32 -lglu32
if %errorlevel% neq 0 (
  echo Build FAILED
) else (
  echo Build OK - run smartroom_v7.exe
)
