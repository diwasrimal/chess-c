#include <assert.h>
#include <stdio.h>

#include "fillers.h"
#include "tools.h"

void fillMovableCells(const Cell touched, Board *b)
{
    // Reset movable cells
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            b->cells[i][j].is_movable = false;

    fillCellsInRange(touched, b);
    filterCellsInRange(touched, b);
}

void fillCellsInRange(const Cell touched, Board *b)
{
    // Reset cells in range
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            b->cells[y][x].in_range = false;    

    enum PieceType ttype = touched.piece.type;
    enum PieceColor tcolor = touched.piece.color;

    switch (ttype) {
    case pawn:
        fillCellsInRangePawn(touched, b);
        break;
    case rook:
    case bishop:
        fillCellsInRangeContinuous(touched, ttype, b);
        break;
    case queen:
        fillCellsInRangeContinuous(touched, rook, b);
        fillCellsInRangeContinuous(touched, bishop, b);
        break;
    case knight:
        fillCellsInRangeKnight(touched, b);
        break;
    case king:
        fillCellsInRangeKing(touched, b);
        break;
    default:
        fprintf(stderr, "Not implemented!\n");
    }

    if (ttype == king) {
        b->queenside_castling_cell[tcolor] = NULL;
        b->kingside_castling_cell[tcolor] = NULL;
        fillCastlingCells(touched, b);
    }
}

void filterCellsInRange(const Cell touched, Board *b)
{
    enum PieceType ttype = touched.piece.type;
    enum PieceColor tcolor = touched.piece.color;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            Cell *cell = &(b->cells[i][j]);
            if (!cell->in_range)
                continue;

            // Filter pieces of same color
            if (cell->piece.color == tcolor) {
                continue;
            }

            // Filter cells dangerous if king is moving
            if (ttype == king && cell->is_dangerous[tcolor]) {
                continue;
            }

            // Filter cells that don't block check when some piece moves there
            if (b->king_checked && b->filter_nonblocking_cells && !touched.check_blocking_cells[i][j]) {
                continue;
            }

            // Filter cells that might open a check to our king
            if (b->filter_check_opening && touched.opens_check && touched.check_opening_cells[i][j])
                continue;

            cell->is_movable = true;
            b->move_pending = true;
        }
    }
}

void fillCellsInRangePawn(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceColor tcolor = touched.piece.color;
    enum PieceType ttype = touched.piece.type;

    if (ttype != pawn)
        assert(0 && "ttype != pawn\n");

    // Direction of move: black goes down, white goes up
    int dir = (tcolor == black) ? 1 : -1;
    int starting_pos = (tcolor == black) ? 1 : 6;
    bool in_starting_position = ti.y == starting_pos;

    // Straight move (should be empty)
    int move_limit = in_starting_position ? 2 : 1;
    for (int dy = 1; dy <= move_limit; dy++) {
        int j = ti.y + dy * dir;
        if (!validCellIdx(ti.x, j) || !emptyCell(b->cells[j][ti.x]))
            break;
        b->cells[j][ti.x].in_range = true;
    }

    // Diagonal moves (captures or en passant)
    int j = ti.y + dir;
    int x_directions[2] = {-1, 1};

    for (int k = 0; k < 2; k++) {
        int x = ti.x + x_directions[k];
        if (!validCellIdx(x, j))
            continue;

        bool capturable = !emptyCell(b->cells[j][x]);
        bool passantable =
            b->has_en_passant_target &&
            b->en_passant_target_idx.y == j &&
            b->en_passant_target_idx.x == x;
        if (capturable || passantable)
            b->cells[j][x].in_range = true;
    }
}


void fillCellsInRangeContinuous(const Cell touched, enum PieceType ttype, Board *b)
{
    V2 ti = touched.idx;

    bool moves_continuous = (ttype == rook || ttype == bishop || ttype == queen);
    if (!moves_continuous)
        assert(0 && "!moves_continuous\n");

    V2 vectors[2][2];
    switch (ttype) {
    case rook:
        vectors[0][0] = (V2){.y = -1, .x = 0};  // up
        vectors[0][1] = (V2){.y = 1, .x = 0};   // down
        vectors[1][0] = (V2){.y = 0, .x = -1};  // left
        vectors[1][1] = (V2){.y = 0, .x = 1};   // right
        break;
    case bishop:
        vectors[0][0] = (V2){.y = -1, .x = -1};  // top left
        vectors[0][1] = (V2){.y = -1, .x = 1};   // top right
        vectors[1][0] = (V2){.y = 1, .x = -1};   // bot left
        vectors[1][1] = (V2){.y = 1, .x = 1};    // bot right
        break;
    default:
        fprintf(stderr, "Not implemented continuous move for type: %d\n", ttype);
    }

    for (int l = 0; l < 2; l++) {
        for (int m = 0; m < 2; m++) {
            V2 vec = vectors[l][m];
            int i = ti.x + vec.x;
            int j = ti.y + vec.y;

            while (validCellIdx(i, j)) {
                if (!emptyCell(b->cells[j][i])) {
                    b->cells[j][i].in_range = true;
                    break;
                }
                b->cells[j][i].in_range = true;
                i += vec.x;
                j += vec.y;
            }
        }
    }
}

void fillCellsInRangeKnight(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceType ttype = touched.piece.type;

    if (ttype != knight)
        assert(0 && "ttype != knight\n");

    int dy[8] = {2, 2, -2, -2, 1, 1, -1, -1};
    int dx[8] = {-1, 1, -1, 1, -2, 2, -2, 2};
    for (int k = 0; k < 8; k++) {
        int i = ti.x + dx[k];
        int j = ti.y + dy[k];
        if (!validCellIdx(i, j)) continue;
        b->cells[j][i].in_range = true;
    }
}

void fillCellsInRangeKing(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceType ttype = touched.piece.type;

    if (ttype != king)
        assert(0 && "ttype != king\n");

    for (int j = ti.y - 1; j <= ti.y + 1; j++) {
        for (int i = ti.x - 1; i <= ti.x + 1; i++) {
            if ((i == ti.x && j == ti.y) || !validCellIdx(i, j))
               continue;
            b->cells[j][i].in_range = true;
        }
    }
}

void fillCastlingCells(const Cell touched, Board *b)
{
    // Cannot castle when king is in check
    if (b->king_checked)
        return;

    V2 ti = touched.idx;
    enum PieceColor tcolor = touched.piece.color;
    enum PieceType ttype = touched.piece.type;

    if (ttype != king)
        assert(0 && "ttype != king, cannot fill castling cells\n");

    bool queenside_empty = emptyCell(b->cells[ti.y][1]) &&
                      emptyCell(b->cells[ti.y][2]) &&
                      emptyCell(b->cells[ti.y][3]);
    bool queenside_attacked = b->cells[ti.y][1].is_dangerous[tcolor] ||
                          b->cells[ti.y][2].is_dangerous[tcolor] ||
                          b->cells[ti.y][3].is_dangerous[tcolor];
    bool kingside_empty = emptyCell(b->cells[ti.y][5]) && emptyCell(b->cells[ti.y][6]);
    bool kingside_attacked = b->cells[ti.y][5].is_dangerous[tcolor] ||
                           b->cells[ti.y][6].is_dangerous[tcolor];

    if (b->queenside_castle_available[tcolor] && queenside_empty && !queenside_attacked) {
        b->cells[ti.y][2].in_range = true;
        b->queenside_castling_cell[tcolor] = &(b->cells[ti.y][2]);
    }
    if (b->kingside_castle_available[tcolor] && kingside_empty && !kingside_attacked) {
        b->cells[ti.y][6].in_range = true;
        b->kingside_castling_cell[tcolor] = &(b->cells[ti.y][6]);
    }
}
