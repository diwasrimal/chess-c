#include <raylib.h>
#include <stdio.h>

#define CELL_SIZE 80
#define BOARD_PADDING 5
#define BOARD_SIZE (CELL_SIZE * 8)
#define WINDOW_SIZE (BOARD_SIZE + BOARD_PADDING * 2)


enum PieceColor {
    black,
    white,
    no_color,
};

enum PieceType {
    king,
    queen,
    bishop,
    knight,
    rook,
    pawn,
    no_type,
};

typedef struct {
    int x;
    int y;
} V2;

typedef struct {
    V2 pos;
    Color bg;
    enum PieceColor piece_color;
    enum PieceType piece_type;
} Cell;

typedef struct {
    Cell cells[8][8];
    Color bg_colors[2];
} Board;


Board initBoard(void);
char *getPieceString(Cell c);
V2 cellPosByIdx(int x, int y);
V2 cellIdxByPos(int pos_x, int pos_y);
