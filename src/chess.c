#include <raylib.h>
#include <stdio.h>

#include "declarations.h"
#include "colorizers.h"
#include "handlers.h"
#include "tools.h"

int main(void)
{
    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    Board board = initBoard();
    PromotionWindow pwin = initPromotionWindow();
    bool draw_debug_hints = false;

    // Load piece textures
    // In order with enums for indexing
    int icon_diff = 14;
    int icon_size = CELL_SIZE - icon_diff;
    Texture2D piece_textures[2][6];
    char *piece_names[] = {"king", "queen", "bishop", "knight", "rook", "pawn"};
    char *piece_colors[] = {"black", "white"};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            char filename[30];
            sprintf(filename, "./resources/%s-%s.png", piece_colors[i], piece_names[j]);
            Image img = LoadImage(filename);
            ImageResize(&img, icon_size, icon_size);
            piece_textures[i][j] = LoadTextureFromImage(img);
            UnloadImage(img);
        }
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(COLOR_BLACK);

        // Take mouse inputs when game is running
        if (!board.checkmate && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (board.promotion_pending)
                handlePromotion(GetMouseX(), GetMouseY(), &board, pwin);
            else
                handleTouch(GetMouseX(), GetMouseY(), &board);
        }

        if (IsKeyPressed(KEY_R)) {
            board = initBoard();
        }

        if (IsKeyPressed(KEY_F)) {
            generateFEN(board);
        }

        // Draw board and pieces
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Cell c = board.cells[y][x];
                DrawRectangle(c.pos.x, c.pos.y, CELL_SIZE, CELL_SIZE, c.bg);

                if (draw_debug_hints) {
                    char idx[4];
                    sprintf(idx, "%d%d", y, x);
                    DrawText(idx, c.pos.x, c.pos.y, 10, BLUE);

                    int bottom_y = c.pos.y + 70;
                    if (c.is_dangerous[white])
                        DrawText("D", c.pos.x + 5, bottom_y, 10, RED);
                    if (c.is_dangerous[black])
                        DrawText("d", c.pos.x + 15, bottom_y, 10, RED);
                    if (c.blocks_check)
                        DrawText("bc", c.pos.x + 30, bottom_y, 10, BLACK);
                    if (c.opens_check)
                        DrawText("pin", c.pos.x + 45, bottom_y, 10, BLACK);
                    if (board.has_en_passant_target &&
                        y == board.en_passant_target_idx.y &&
                        x == board.en_passant_target_idx.x)
                        DrawText("ep", c.pos.x + 60, bottom_y, 10, BLACK);
                }

                if (emptyCell(c))
                    continue;

                // Draw textures of chess pieces
                DrawTexture(piece_textures[c.piece.color][c.piece.type],
                            c.pos.x + icon_diff / 2, c.pos.y + icon_diff / 2,
                            COLOR_WHITE);
            }
        }

        // Draw a window to select promoted piece if promotion is pending
        if (board.promotion_pending) {
            enum PieceColor promoting_color = board.promoting_cell->piece.color;

            DrawRectangle(pwin.pos.x, pwin.pos.y, pwin.width, pwin.height,
                          COLOR_BLACK);
            DrawText(pwin.text,
                     pwin.pos.x + pwin.width / 2 - pwin.text_width / 2,
                     pwin.pos.y + pwin.padding, pwin.text_height, COLOR_WHITE);

            int posy = pwin.first_cell_pos.y;
            for (int i = 0; i < 4; i++) {
                int posx = pwin.first_cell_pos.x + i * (CELL_SIZE + pwin.cell_margin);
                Piece p = { .type = pwin.promotables[i], .color = promoting_color};

                DrawRectangle(posx, pwin.first_cell_pos.y, CELL_SIZE, CELL_SIZE, checkers[i % 2]);
                DrawTexture(piece_textures[p.color][p.type],
                            posx + icon_diff / 2, posy + icon_diff / 2,
                            COLOR_WHITE);
            }
        }

        // Show a red CHECKMATE during checkmate
        if (board.checkmate) {
            char *text = "Checkmate!";
            int size = 50;
            int width = MeasureText(text, size);
            DrawText(text, BOARD_SIZE / 2 - width / 2, BOARD_SIZE / 2 - size / 2, size, RED);
        }

        // Show draw during draw
        bool drawn = board.draw_by_fifty_move || board.draw_by_stalemate;
        if (drawn) {
            int size = 50;
            char *text = board.draw_by_fifty_move  ? "Draw (fifty move rule)"
                         : board.draw_by_stalemate ? "Draw (stalemate)"
                                                   : "Draw";
            int width = MeasureText(text, size);
            DrawText(text, BOARD_SIZE / 2 - width / 2, BOARD_SIZE / 2 - size / 2, size, BLUE);
        }

        EndDrawing();
    }

    // Unload textures
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            UnloadTexture(piece_textures[i][j]);
        }
    }

    CloseWindow();
}
