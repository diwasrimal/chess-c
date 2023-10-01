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

typedef struct {
    bool possible[8][8];
    bool valid[8][8];
    bool move_pending;
    Cell *active_cell;
} MoveHandler;


Board initBoard(void);
MoveHandler initMoveHandler(void);
char *getPieceString(Cell c);
V2 cellPosByIdx(int x, int y);
V2 cellIdxByPos(int pos_x, int pos_y);
void resetCellBackgrounds(Board *b);
void handleMove(int mouse_x, int mouse_y, Board *b, MoveHandler *h);
bool validCellIdx(int x, int y);
void fillPossibleMovesPawn(int x, int y, bool isBlack, MoveHandler *h, const Board *b);
void fillPossibleMovesContinuous(int x, int y, enum PieceType t, MoveHandler *h, const Board *b);
void fillPossibleMovesKnight(int x, int y, MoveHandler *h, const Board *b);
void fillPossibleMovesKing(int x, int y, MoveHandler *h, const Board *b);
