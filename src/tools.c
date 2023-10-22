#include <assert.h>
#include <stddef.h>

#include "tools.h"
#include "recorders.h"
#include "colorizers.h"

Board initBoard(void)
{
    Board b;

    resetCellBackgrounds(&b);
    b.turn = white;
    b.move_count = 0;
    b.last_move = (Move){.src = NULL, .dst = NULL};
    b.move_pending = false;
    b.promotion_pending = false;
    b.checkmate = false;
    b.king_checked = false;
    b.filter_nonblocking_cells = true;
    b.filter_check_opening = true;
    b.left_castle_possible[black] = true;
    b.left_castle_possible[white] = true;
    b.right_castle_possible[black] = true;
    b.right_castle_possible[white] = true;
    b.checked_king = NULL;
    b.active_cell = NULL;
    b.promoting_cell = NULL;
    b.left_castling_cell[black] = NULL;
    b.left_castling_cell[white] = NULL;
    b.right_castling_cell[black] = NULL;
    b.right_castling_cell[white] = NULL;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b.cells[y][x].pos = cellPosByIdx(x, y);
            b.cells[y][x].idx.y = y;
            b.cells[y][x].idx.x = x;
            b.cells[y][x].in_range = false;
            b.cells[y][x].is_movable = false;
            b.cells[y][x].is_dangerous[black] = false;
            b.cells[y][x].is_dangerous[white] = false;
            b.cells[y][x].blocks_check = false;
            b.cells[y][x].opens_check = false;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    b.cells[y][x].check_blocking_cells[i][j] = false;
                    b.cells[y][x].check_opening_cells[i][j] = false;
                }
            }
        }
    }

    enum PieceType main_row[] = {rook, knight, bishop, queen, king, bishop, knight, rook};

    // Black pieces (top)
    for (int i = 0; i < 8; i++) {
        b.cells[0][i].piece.color = black;
        b.cells[0][i].piece.type = main_row[i];
        b.cells[1][i].piece.color = black;
        b.cells[1][i].piece.type = pawn;
    }

    // Empty pieces
    Piece empty_piece = {.color = no_color, .type = no_type};
    for (int i = 2; i < 6; i++)
        for (int j = 0; j < 8; j++)
            b.cells[i][j].piece = empty_piece;

    // White pieces (bottom)
    for (int i = 0; i < 8; i++) {
        b.cells[6][i].piece.color = white;
        b.cells[6][i].piece.type = pawn;
        b.cells[7][i].piece.color = white;
        b.cells[7][i].piece.type = main_row[i];
    }

    // Record dangerous cells for opponent
    recordDangerousCells(&b);

    return b;
}

PromotionWindow initPromotionWindow(void)
{
    PromotionWindow pwin;
    pwin.promotables[0] = queen;
    pwin.promotables[1] = rook;
    pwin.promotables[2] = knight;
    pwin.promotables[3] = bishop;

    pwin.padding = 10;
    pwin.cell_margin = 5;

    pwin.text = "Promote To";
    pwin.text_height = 30;
    pwin.text_width = MeasureText(pwin.text, pwin.text_height);

    pwin.height = CELL_SIZE + pwin.padding * 3 + pwin.text_height;
    pwin.width = CELL_SIZE * 4 + pwin.cell_margin * 3 + pwin.padding * 2;

    pwin.pos.x = BOARD_SIZE / 2 - pwin.width / 2;
    pwin.pos.y = BOARD_SIZE / 2 - pwin.height / 2;
    pwin.first_cell_pos.x = pwin.pos.x + pwin.padding;
    pwin.first_cell_pos.y = pwin.pos.y + pwin.padding * 2 + pwin.text_height;

    return pwin;
}

char *getPieceTypeString(Piece p)
{
    switch (p.type) {
    case rook:
        return "R";
    case knight:
        return "Kn";
    case bishop:
        return "B";
    case king:
        return "K";
    case queen:
        return "Q";
    case pawn:
        return "P";
    case no_type:
        return NULL;
    }
    return NULL;
}

// Returns the drawing position
V2 cellPosByIdx(int x, int y)
{
    V2 vec;
    vec.x = BOARD_PADDING + (x * CELL_SIZE);
    vec.y = BOARD_PADDING + (y * CELL_SIZE);
    return vec;
}

V2 cellIdxByPos(int pos_x, int pos_y)
{
    V2 vec;
    vec.x = (pos_x - BOARD_PADDING) / CELL_SIZE;
    vec.y = (pos_y - BOARD_PADDING) / CELL_SIZE;
    return vec;
}

bool validCellIdx(int x, int y)
{
    return (0 <= x && x < 8) && (0 <= y && y < 8);
}

bool emptyCell(Cell c)
{
    return c.piece.type == no_type && c.piece.color == no_color;
}

void movePiece(Cell *from, Cell *to)
{
    if (from == NULL || to == NULL)
        assert(0 && "Cannot make move, from == NULL || to == NULL\n");

    Piece empty_piece = {.color = no_color, .type = no_type};
    to->piece = from->piece;
    from->piece = empty_piece;
}

void makeMove(const Move move, Board *b)
{
    enum PieceColor scolor = move.src->piece.color;
    enum PieceType stype = move.src->piece.type;
    V2 di = move.dst->idx;

    recordCastlingPossibility(move, b);
    movePiece(move.src, move.dst);
    b->last_move = move;
    b->move_count++;

    bool castled =
        stype == king && (move.dst == b->left_castling_cell[scolor] ||
                          move.dst == b->right_castling_cell[scolor]);
    if (castled) {
        int rook_dir = (move.dst == b->left_castling_cell[scolor]) ? 1 : -1;
        int rook_x = (move.dst == b->left_castling_cell[scolor]) ? 0 : 7;
        movePiece(&(b->cells[di.y][rook_x]), &(b->cells[di.y][di.x + rook_dir]));
        b->left_castle_possible[scolor] = false;
        b->right_castle_possible[scolor] = false;
    }

    b->move_pending = false;
    b->active_cell = NULL;
    changeTurn(b);

    // Handle promotions of pawns after moving if possible
    if (stype == pawn) {
        int promoting_y = (scolor == black) ? 7 : 0;
        if (move.dst->idx.y == promoting_y) {
            b->promotion_pending = true;
            b->promoting_cell = move.dst;
            return;
        }
    }

    recordDangerousCells(b);
    recordPins(b, b->turn);
    recordCheck(b);
}

void changeTurn(Board *b)
{
    b->turn = b->turn == black ? white : black;
}
