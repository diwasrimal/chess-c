#ifndef TOOLS_H
#define TOOLS_H

#include "declarations.h"
#include "recorders.h"

Board initBoard(void);
Board initBoardFromFEN(char *fen);
PromotionWindow initPromotionWindow(void);
void generateFEN(Board b);
V2 cellPosByIdx(int x, int y);
V2 cellIdxByPos(int pos_x, int pos_y);
bool validCellIdx(int x, int y);
bool emptyCell(Cell c);
void movePiece(Cell *from, Cell *to);
void makeMove(const Move m, Board *b);
void changeTurn(Board *b);

#endif // TOOLS_H
