#!/bin/sh

set -xe

LIBS=$(pkg-config --libs raylib)
CFLAGS="-Wall -Wextra $(pkg-config --cflags raylib)"
CC=clang

$CC $CFLAGS -g -o chess chess.c $LIBS