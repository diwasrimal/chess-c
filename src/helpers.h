#include <raylib.h>
#include <stdio.h>
#include <assert.h>

#define CELL_SIZE               80
#define BOARD_PADDING           10
#define BOARD_SIZE              (CELL_SIZE * 8)
#define WINDOW_SIZE             (BOARD_SIZE + BOARD_PADDING * 2)
#define COLOR_RED               (Color){0xd7, 0x6c, 0x6c, 0xff}
#define COLOR_BLACK             (Color){0x4E, 0x53, 0x56, 0xff}
#define COLOR_GREEN             (Color){0x50, 0xfa, 0x7b, 0xff}
#define COLOR_GREY              (Color){0xc7, 0xce, 0xd1, 0xff}
#define COLOR_WHITE             WHITE
#define COLOR_CELL_ACTIVE       (Color){0xd7, 0xc8, 0x6c, 0xff}
#define COLOR_CELL_MOVABLE      (Color){0x8e, 0xb7, 0xd6, 0xff}
#define COLOR_CELL_CASTLING     (Color){0x5e, 0x81, 0xac, 0xff}
#define COLOR_CELL_CAPTURABLE   COLOR_RED
#define COLOR_MOVE_SRC          (Color){0xcb, 0xdd, 0xaf, 0xff}
#define COLOR_MOVE_DST          COLOR_MOVE_SRC
#define COLOR_CHECKER_DARK      COLOR_GREY
#define COLOR_CHECKER_LIGHT     COLOR_WHITE


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
    enum PieceType type;
    enum PieceColor color;
} Piece;

typedef struct {
    V2 pos;
    V2 idx;
    Color bg;
    Piece piece;
    bool is_dangerous[2];   // dangerous for black or white king
    bool in_range;
    bool is_movable;
    bool blocks_check;      // Cell can block check
    bool opens_check;       // Check happens if piece on cell moves somewhere (pin)
    bool check_opening_cells[8][8];     // Moving on one of these will open check
    bool check_blocking_cells[8][8];    // Moving on one of these will block check
} Cell;

typedef struct {
    Cell *src;
    Cell *dst;
} Move;

typedef struct {
    Cell cells[8][8];
    Cell *active_cell;
    Cell *checked_king;
    Cell *promoting_cell;
    Cell *left_castling_cell[2];
    Cell *right_castling_cell[2];
    Move last_move;
    bool king_checked;
    bool move_pending;
    bool promotion_pending;
    bool checkmate;
    bool filter_nonblocking_cells;      // filter out cells that don't help block check
    bool filter_check_opening;          // filter out cells that open check
    bool left_castle_possible[2];
    bool right_castle_possible[2];
    enum PieceColor turn;
    unsigned int move_count;
} Board;

typedef struct {
    enum PieceType promotables[4];
    V2 pos;
    V2 first_cell_pos;
    int height;
    int width;
    int padding;
    int cell_margin;
    int text_height;
    int text_width;
    char *text;
} PromotionWindow;

Color checkers[2] = {COLOR_CHECKER_DARK, COLOR_CHECKER_LIGHT};

Board initBoard(void);
PromotionWindow initPromotionWindow(void);
char *getPieceTypeString(Piece p);
V2 cellPosByIdx(int x, int y);
V2 cellIdxByPos(int pos_x, int pos_y);
void resetCellBackgrounds(Board *b);
void handleTouch(int mouse_x, int mouse_y, Board *b);
void handlePromotion(int mouse_x, int mouse_y, Board *b, const PromotionWindow pwin);
void movePiece(Cell *from, Cell *to);
void makeMove(const Move m, Board *b);
bool validCellIdx(int x, int y);
bool emptyCell(Cell c);
void changeTurn(Board *b);
void recolorCell(Cell *cell, Color color);
void fillMovableCells(const Cell touched, Board *b);
void colorMovableCells(const Cell touched, Board *b);
void fillCellsInRange(const Cell touched, Board *b);
void fillCellsInRangePawn(const Cell touched, Board *b);
void fillCellsInRangeContinuous(const Cell touched, enum PieceType ttype, Board *b);
void fillCellsInRangeKnight(const Cell touched, Board *b);
void fillCellsInRangeKing(const Cell touched, Board *b);
void fillCastlingCells(const Cell touched, Board *b);
void filterCellsInRange(const Cell touched, Board *b);
void recordCastlingPossibility(Move m, Board *b);
void recordDangerousCells(Board *b);
void recordDangerousCellsByPawn(int x, int y, Board *b);
void recordCheck(Board *b);
void recordPins(Board *b, enum PieceColor color);