#ifndef HANDLERS_H
#define HANDLERS_H

#include "declarations.h"

void handleTouch(int mouse_x, int mouse_y, Board *b);
void handlePromotion(int mouse_x, int mouse_y, Board *b, const PromotionWindow pwin);
void *handleComputerTurn(void *board);

#endif // HANDLERS_H
