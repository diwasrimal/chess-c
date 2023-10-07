CFLAGS = -Wall -Wextra -g `pkg-config --cflags raylib`
LIBS = `pkg-config --libs raylib`
CC = clang

chess: src/chess.c src/helpers.c src/helpers.h
	$(CC) $(CFLAGS) -o chess src/chess.c $(LIBS)
