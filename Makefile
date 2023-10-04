CFLAGS = -Wall -Wextra -g `pkg-config --cflags raylib`
LIBS = `pkg-config --libs raylib`
CC = clang

chess: chess.c helpers.c helpers.h
	$(CC) $(CFLAGS) -o chess chess.c $(LIBS)