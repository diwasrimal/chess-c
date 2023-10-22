#ifndef FILLERS_H
#define FILLERS_H

#include "declarations.h"

void fillMovableCells(const Cell touched, Board *b);
void fillCellsInRange(const Cell touched, Board *b);
void fillCellsInRangePawn(const Cell touched, Board *b);
void fillCellsInRangeContinuous(const Cell touched, enum PieceType ttype, Board *b);
void fillCellsInRangeKnight(const Cell touched, Board *b);
void fillCellsInRangeKing(const Cell touched, Board *b);
void fillCastlingCells(const Cell touched, Board *b);
void filterCellsInRange(const Cell touched, Board *b);

#endif // FILLERS_H