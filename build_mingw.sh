#!/bin/sh

set -xe

target=${1:-chess.exe}

LIBS="-Lraylib-4.5.0_win64_mingw-w64/lib/ -lraylib -lwinmm -lgdi32 -lopengl32"
CFLAGS="-g -Wall -Wextra -static -Iraylib-4.5.0_win64_mingw-w64/include/"
CC=x86_64-w64-mingw32-gcc

$CC $CFLAGS -o "$target" src/chess.c src/colorizers.c src/fillers.c src/handlers.c src/recorders.c src/tools.c $LIBS
