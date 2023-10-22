#include "colors.h"
#include "tools.h"

Color checkers[2] = {COLOR_CHECKER_DARK, COLOR_CHECKER_LIGHT};

void resetCellBackgrounds(Board *b)
{
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            b->cells[y][x].bg = checkers[(y + x) % 2];
}

void colorMovableCells(const Cell touched, Board *b)
{
    enum PieceColor tcolor = touched.piece.color;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Cell *cell = &(b->cells[i][j]);
            if (!cell->is_movable)
                continue;

            Color newbg = !emptyCell(*cell) ? COLOR_CELL_CAPTURABLE : COLOR_CELL_MOVABLE;
            if (cell == b->left_castling_cell[tcolor] || cell == b->right_castling_cell[tcolor])
                newbg = COLOR_CELL_CASTLING;

            recolorCell(cell, newbg);
        }
    }
}

void colorKingIfChecked(Board *b)
{
    if (b->king_checked)
        recolorCell(b->checked_king, COLOR_CELL_CAPTURABLE);
}

void decolorKingIfChecked(Board *b)
{
    if (b->king_checked) {
        Cell *ck = b->checked_king;
        ck->bg = checkers[(ck->idx.y + ck->idx.x) % 2];
    }
}

void colorLastMove(Board *b)
{
    if (b->move_count == 0)
        return;
    recolorCell(b->last_move.src, COLOR_MOVE_SRC);
    recolorCell(b->last_move.dst, COLOR_MOVE_DST);
}

void decolorLastMove(Board *b)
{
    if (b->move_count == 0)
        return;
    Cell *src = b->last_move.src;
    Cell *dst = b->last_move.dst;
    src->bg = checkers[(src->idx.y + src->idx.x) % 2];
    dst->bg = checkers[(dst->idx.y + dst->idx.x) % 2];
}

// Applies a color on top of an exisiting background color
void recolorCell(Cell *cell, Color color)
{
    Color original = checkers[(cell->idx.y + cell->idx.x) % 2];
    bool cell_is_dark =
        original.r == COLOR_CHECKER_DARK.r &&
        original.g == COLOR_CHECKER_DARK.g &&
        original.b == COLOR_CHECKER_DARK.b;

    color.a = cell_is_dark ? 255 : 200;
    cell->bg = color;
}