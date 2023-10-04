#!/bin/sh

set -xe

target=$1
[ -z "$target" ] && target="chess"

LIBS=$(pkg-config --libs raylib)
CFLAGS="-g -Wall -Wextra $(pkg-config --cflags raylib)"
CC=clang

$CC $CFLAGS -o "$target" chess.c $LIBS