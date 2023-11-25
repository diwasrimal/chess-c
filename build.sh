#!/bin/sh

set -xe

target=${1:-chess}
LIBS="$(pkg-config --libs raylib)"
CFLAGS="-O3 -Wall -Wextra $(pkg-config --cflags raylib)"
CC=clang

$CC $CFLAGS -o "$target" src/chess.c src/colorizers.c src/fillers.c src/handlers.c src/recorders.c src/tools.c $LIBS
