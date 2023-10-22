#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <raylib.h>

#define CELL_SIZE               80
#define BOARD_PADDING           10
#define BOARD_SIZE              (CELL_SIZE * 8)
#define WINDOW_SIZE             (BOARD_SIZE + BOARD_PADDING * 2)

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

#endif // DECLARATIONS_H