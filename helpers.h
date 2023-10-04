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
    enum PieceColor color;
    enum PieceType type;
    bool is_dangerous[2];   // dangerous for black or white king
    bool in_range;
    bool is_movable;
    bool blocks_check;
    bool check_blocking_cells[8][8];
} Cell;

typedef struct {
    Cell cells[8][8];
    Cell *active_cell;
    Cell *checked_king;
    bool king_checked;
    bool move_pending;
    bool checkmate;
    bool filter_nonblocking_cells;  // filter out cells that don't help block check
    enum PieceColor turn;
} Board;

Board initBoard(void);
char *getPieceString(Cell c);
V2 cellPosByIdx(int x, int y);
V2 cellIdxByPos(int pos_x, int pos_y);
void resetCellBackgrounds(Board *b);
void handleMove(int mouse_x, int mouse_y, Board *b);
bool validCellIdx(int x, int y);
void fillPossibleMovesPawn(int x, int y, Board *b);
void fillPossibleMovesContinuous(int x, int y, enum PieceType t, Board *b);
void fillPossibleMovesKnight(int x, int y, Board *b);
void fillPossibleMovesKing(int x, int y, Board *b);
void recordDangerousCells(Board *b);
void recordDangerousCellsByPawn(int x, int y, Board *b);
void recordCheck(Board *b);
bool emptyCell(Cell c);
