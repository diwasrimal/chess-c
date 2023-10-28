#ifndef RECORDERS_H
#define RECORDERS_H

#include "declarations.h"

void recordCastlingRightChanges(Move m, Board *b);
void recordDangerousCells(Board *b);
void recordDangerousCellsByPawn(int x, int y, Board *b);
void recordCheck(Board *b);
void recordPins(Board *b, enum PieceColor color);
void recordDraw(Board *b);

#endif // RECORDERS_H