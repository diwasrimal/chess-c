CFLAGS = -Wall -Wextra -g `pkg-config --cflags raylib`
LIBS = `pkg-config --libs raylib` -lpthread
CC = clang
SOURCE = src/chess.c src/colorizers.c src/fillers.c src/handlers.c src/recorders.c src/tools.c
HEADERS = src/declarations.h src/colorizers.h  src/fillers.h src/handlers.h src/recorders.h src/tools.h

chess: $(SOURCE) $(HEADERS)
	$(CC) $(CFLAGS) -o chess $(SOURCE) $(LIBS)

clean:
	rm -f chess