@echo off
REM Copyright (c) 2024, Ivan Reshetnikov - All rights reserved.
REM MSVC CLI docs: https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170


:: Flags for compilation
set "FLAGS="

:: C version (ISO/IEC 9899:2011)
set "FLAGS=%FLAGS% /std:c++17"
:: Max info level
set "FLAGS=%FLAGS% /W3"
:: Max optimization (favor speed)
set "FLAGS=%FLAGS% /O2"
:: Max optimization (favor output size)
:: set "FLAGS=%FLAGS% /O1"

set "SOURCE_FILES=./src/glad.c ./src/main.cpp ./src/camera.cpp"
set "OUT_FILENAME=./bin/main.exe"

del %OUT_FILENAME%

cl %FLAGS% /I"include" %SOURCE_FILES% /Fo"obj/" /EHsc /link /LIBPATH:"lib/" glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib msvcrt.lib /out:%OUT_FILENAME%
