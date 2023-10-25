#include <stdio.h>
#include <stddef.h>

#include "recorders.h"
#include "fillers.h"
#include "tools.h"

// Records changes in castling rights when a move is made
void recordCastlingRightChanges(Move m, Board *b)
{
    V2 si = m.src->idx;
    V2 di = m.dst->idx;
    enum PieceType stype = m.src->piece.type;
    enum PieceType dtype = m.dst->piece.type;
    enum PieceColor scolor = m.src->piece.color;
    enum PieceColor dcolor = m.dst->piece.color;

    // King moves
    if (stype == king) {
        b->queenside_castle_available[scolor] = false;
        b->kingside_castle_available[scolor] = false;
    }

    // Rook moves
    if (stype == rook && si.x == 0)
        b->queenside_castle_available[scolor] = false;
    if (stype == rook && si.x == 7)
        b->kingside_castle_available[scolor] = false;

    // Rook capture
    if (dtype == rook && di.x == 0)
        b->queenside_castle_available[dcolor] = false;
    if (dtype == rook && di.x == 7)
        b->kingside_castle_available[dcolor] = false;
}

// Record cells that will become dangerous to opponent
void recordDangerousCells(Board *b)
{
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].is_dangerous[black] = false;
            b->cells[y][x].is_dangerous[white] = false;
        }
    }

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell c = b->cells[y][x];
            if (emptyCell(c))
                continue;

            enum PieceColor opposing = (c.piece.color == black) ? white : black;

            // Handle pawns differently
            if (c.piece.type == pawn) {
                recordDangerousCellsByPawn(x, y, b);
                continue;
            }

            // Simulate touching the piece using a temporary board
            // Find cells that are in range, those cells will be dangerous
            // for opponent to enter
            Board tmp = *b;
            tmp.turn = c.piece.color;
            tmp.move_pending = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    tmp.cells[i][j].in_range = false;

            fillCellsInRange(c, &tmp);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (tmp.cells[i][j].in_range)
                        b->cells[i][j].is_dangerous[opposing] = true;
                }
            }
        }
    }
}

void recordDangerousCellsByPawn(int x, int y, Board *b)
{
    // Pawn captures only diagonals, thus threatens only diagonals
    Cell touched = b->cells[y][x];
    int dir = (touched.piece.color == black) ? 1 : -1;
    int j = y + dir;
    int xl = x - 1;
    int xr = x + 1;
    enum PieceColor dangerous_for = touched.piece.color == black ? white : black;
    if (validCellIdx(xl, j))
        b->cells[j][xl].is_dangerous[dangerous_for] = true;
    if (validCellIdx(xr, j))
        b->cells[j][xr].is_dangerous[dangerous_for] = true;
}


void recordPins(Board *b, enum PieceColor color)
{
    // Reset
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].opens_check = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    b->cells[y][x].check_opening_cells[i][j] = false;
        }
    }

    // Make a copy of board
    Board copy = *b;
    copy.turn = color;

    // Locate king's position
    V2 king_idx;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell c = b->cells[y][x];
            if (c.piece.type == king && c.piece.color == color) {
                king_idx.x = x;
                king_idx.y = y;
                break;
            }
        }
    }

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            Cell src = copy.cells[y][x];
            if (emptyCell(src) || src.piece.color != color || src.piece.type == king)
                continue;

            // Collect movable moves (dont filter check opening cells)
            // Otherwise everything may get filtered in first try
            Board tmp1 = copy;
            tmp1.filter_check_opening = false;
            tmp1.move_pending = false;
            fillMovableCells(src, &tmp1);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    Cell dst = tmp1.cells[i][j];
                    if (!dst.is_movable)
                        continue;

                    Board tmp2 = tmp1;
                    movePiece(&tmp2.cells[y][x], &tmp2.cells[i][j]);
                    recordDangerousCells(&tmp2);

                    // If king is in danger after moving, cell will open check to king
                    if (tmp2.cells[king_idx.y][king_idx.x].is_dangerous[color]) {
                        b->cells[y][x].opens_check = true;
                        b->cells[y][x].check_opening_cells[i][j] = true;
                        b->filter_check_opening = true;     // filter out check opening cells in further moves
                    }
                }
            }
        }
    }
    return;
}

// Finds whether king is checked, and finds cells that can block the check
void recordCheck(Board *b)
{
    enum PieceColor king_color = b->turn;
    V2 king_idx;
    b->king_checked = false;
    b->filter_nonblocking_cells = false;
    b->checked_king = NULL;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].blocks_check = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    b->cells[y][x].check_blocking_cells[i][j] = false;
        }
    }

    // Find if king checked
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Cell c = b->cells[i][j];
            if (c.piece.type == king && c.piece.color == king_color && c.is_dangerous[king_color]) {
                b->king_checked = true;
                b->checked_king = &(b->cells[i][j]);
                b->filter_nonblocking_cells = true;
                king_idx.y = i;
                king_idx.x = j;
            }
        }
    }

    if (!b->king_checked)
        return;

    // Find cells that will block the check
    // Simulate moving all pieces to each of their movable cells
    // If that makes the king safe, that moved cell blocks the check
    bool can_be_blocked = false;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            Cell src = b->cells[y][x];
            if (emptyCell(src))
                continue;

            if (src.piece.color != king_color)
                continue;

            // Collect movable moves (dont filter non blocking moves)
            // Otherwise everything may get filtered in first try
            Board tmp1 = *b;
            tmp1.move_pending = false;
            tmp1.filter_nonblocking_cells = false;
            fillMovableCells(src, &tmp1);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {

                    Cell dst = tmp1.cells[i][j];
                    if (!dst.is_movable)
                        continue;

                    Board tmp2 = tmp1;
                    movePiece(&tmp2.cells[y][x], &tmp2.cells[i][j]);
                    recordDangerousCells(&tmp2);

                    // If king is safe now, src blocks the check
                    // If src was king, king is at (i, j) now
                    V2 king_at = (src.piece.type == king) ? (V2){.y = i, .x = j} : king_idx;
                    bool king_safe = !tmp2.cells[king_at.y][king_at.x].is_dangerous[king_color];
                    if (king_safe) {
                        can_be_blocked = true;
                        b->cells[y][x].blocks_check = true;
                        b->cells[y][x].check_blocking_cells[i][j] = true;
                    }
                }
            }
        }
    }

    if (!can_be_blocked)
        b->checkmate = true;
}