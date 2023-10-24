#include <stdio.h>
#include <assert.h>

#include "handlers.h"
#include "colorizers.h"
#include "tools.h"
#include "fillers.h"

void handleTouch(int mouse_x, int mouse_y, Board *b)
{
    // Always color these
    resetCellBackgrounds(b);
    colorKingIfChecked(b);
    colorLastMove(b);

    V2 ti = cellIdxByPos(mouse_x, mouse_y);     // touched idx
    Cell *touched = &(b->cells[ti.y][ti.x]);
    enum PieceColor tcolor = touched->piece.color;

    if (b->move_pending) {
        if (touched->is_movable) {
            Move move = {.src = b->active_cell, .dst = touched};
            decolorKingIfChecked(b);
            decolorLastMove(b);
            makeMove(move, b);
            colorLastMove(b);
            colorKingIfChecked(b);
            return;
        }
        else {
            b->move_pending = false;
        }
    }

    if (tcolor != b->turn)
        return;

    if (emptyCell(*touched))
        return;

    recolorCell(touched, COLOR_CELL_ACTIVE);
    b->active_cell = touched;
    b->move_pending = false;

    fillMovableCells(*touched, b);
    colorMovableCells(*touched, b);
}

void handlePromotion(int mouse_x, int mouse_y, Board *b, const PromotionWindow pwin)
{
    if (!b->promotion_pending)
        assert(0 && "!b->promotion_pending\n");

    int fx = pwin.first_cell_pos.x;
    int fy = pwin.first_cell_pos.y;
    int lx = fx + pwin.cell_margin * 3 + CELL_SIZE * 4;
    int ly = fy + CELL_SIZE;

    // Touched outside promotion choosing window
    if (mouse_x < fx || mouse_x > lx || mouse_y < fy || mouse_y > ly)
        return;

    int idx = (mouse_x - fx) / CELL_SIZE;
    Piece chosen = {.type = pwin.promotables[idx], .color = b->promoting_cell->piece.color};
    b->promoting_cell->piece = chosen;
    b->promotion_pending = false;
    b->promoting_cell = NULL;

    recordDangerousCells(b);
    recordPins(b, b->turn);
    recordCheck(b);
}