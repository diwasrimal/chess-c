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
    // bool draw_debug_hints = true;
    // bool use_textures = false;
    bool draw_debug_hints = false;
    bool use_textures = true;

    // Load piece textures
    // In order with enums for indexing
    int icon_diff = 15;
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

        // Handle clicks / key presses
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !board.checkmate) {
            if (board.promotion_pending)
                handlePromotion(GetMouseX(), GetMouseY(), &board, pwin);
            else
                handleTouch(GetMouseX(), GetMouseY(), &board);
        }

        // if (IsKeyPressed(KEY_R)) {
        //     board = initBoard();
        // }

        if (IsKeyPressed(KEY_F)) {
            generateFEN(board);
        }

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Cell c = board.cells[y][x];
                DrawRectangle(c.pos.x, c.pos.y, CELL_SIZE, CELL_SIZE, c.bg);

                if (draw_debug_hints) {
                    if (c.is_dangerous[white])
                        DrawText("DW", c.pos.x + 40, c.pos.y + 10, 12, RED);
                    if (c.is_dangerous[black])
                        DrawText("DB", c.pos.x + 40, c.pos.y + 30, 12, RED);
                    if (c.blocks_check)
                        DrawText("CBS", c.pos.x + 40, c.pos.y + 50, 10, BLACK);
                    if (c.opens_check)
                        DrawText("PIN", c.pos.x + 40, c.pos.y + 70, 10, BLACK);
                }

                if (emptyCell(c))
                    continue;

                // Draw textures or text for icons
                if (use_textures) {
                    DrawTexture(piece_textures[c.piece.color][c.piece.type],
                                c.pos.x + icon_diff / 2,
                                c.pos.y + icon_diff / 2,
                                COLOR_WHITE);
                }
                else {
                    const char *s = getPieceTypeString(c.piece);
                    DrawText(s, c.pos.x + 10, c.pos.y + 10, 20,
                             c.piece.color == black ? BLACK : COLOR_GREEN);
                }
            }
        }

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
                if (use_textures) {
                    DrawTexture(piece_textures[p.color][p.type],
                                posx + icon_diff / 2,
                                posy + icon_diff / 2,
                                COLOR_WHITE);
                }
                else {
                    const char *s = getPieceTypeString(p);
                    DrawText(s, posx + 10, posy + 10, 20, (promoting_color == black) ? COLOR_BLACK : COLOR_GREEN);
                }
            }
        }

        if (board.checkmate) {
            char *text = "Checkmate!";
            int size = 50;
            int width = MeasureText(text, size);
            DrawText(text, BOARD_SIZE / 2 - width / 2, BOARD_SIZE / 2 - size / 2, size, RED);
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