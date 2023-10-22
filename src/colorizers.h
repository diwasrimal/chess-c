#ifndef COLORS_H
#define COLORS_H

#include "declarations.h"

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

extern Color checkers[2];

void resetCellBackgrounds(Board *b);
void colorMovableCells(const Cell touched, Board *b);
void colorKingIfChecked(Board *b);
void colorLastMove(Board *b);
void recolorCell(Cell *cell, Color color);
void decolorKingIfChecked(Board *b);
void decolorLastMove(Board *b);

#endif // COLORS_H