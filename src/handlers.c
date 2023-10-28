#include <limits.h>
#include <stdio.h>
#include <assert.h>
// #include <sys/time.h>

#include "handlers.h"
#include "colorizers.h"
#include "recorders.h"
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
    recordDraw(b);
    colorKingIfChecked(b);
}

void *handleComputerTurn(void *board)
{
    // struct timeval t1, t2;
    // double elapsedTime;
    // gettimeofday(&t1, NULL);
    Board *b = board;
    printf("Computer thinking...\n");

    bool is_maximizing = b->turn == white;
    float best_score = is_maximizing ? INT_MIN : INT_MAX;
    Move best_move = {.src = NULL, .dst = NULL};
    int alpha = INT_MIN;
    int beta = INT_MAX;
    int minimax_depth = 3;  // >= 1

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell src = b->cells[y][x];
            if (src.piece.color != b->turn || emptyCell(src))
                continue;

            Board unmoved = *b;
            unmoved.move_pending = false;
            fillMovableCells(src, &unmoved);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    Cell dst = unmoved.cells[i][j];
                    if (!dst.is_movable)
                        continue;

                    // fprintf(log_file, "%d%d -> %d%d:\n", y, x, i, j);

                    Board moved = unmoved;
                    moved.move_pending = true;
                    Move m = {.src = &(moved.cells[y][x]), .dst = &(moved.cells[i][j])};
                    makeMove(m, &moved);

                    float score = minimax(moved, minimax_depth - 1, !is_maximizing, alpha, beta);
                    // fprintf(log_file, "score: %f\n", score);

                    if (is_maximizing) {
                        if (score > best_score) {
                            best_score = score;
                            best_move.src = &(b->cells[y][x]);
                            best_move.dst = &(b->cells[i][j]);
                        }
                    }
                    else {
                        if (score < best_score) {
                            best_score = score;
                            best_move.src = &(b->cells[y][x]);
                            best_move.dst = &(b->cells[i][j]);
                        }
                    }
                }
            }
        }
    }

    // gettimeofday(&t2, NULL);
    // elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    // elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    // printf("time elapsed: %f ms.\n", elapsedTime);

    if (best_move.src == NULL || best_move.dst == NULL) {
        b->checkmate = true;
        return NULL;
    }

    decolorKingIfChecked(b);
    decolorLastMove(b);
    makeMove(best_move, b);
    colorLastMove(b);
    colorKingIfChecked(b);
    b->computer_thinking = false;
    return NULL;
}