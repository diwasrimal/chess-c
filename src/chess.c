#include <raylib.h>
#include <stdio.h>
#include <pthread.h>

#include "declarations.h"
#include "colorizers.h"
#include "handlers.h"
#include "tools.h"

FILE *log_file;

int main(void)
{
    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    log_file = fopen("log.txt", "w");
    Board board = initBoard();
    PromotionWindow pwin = initPromotionWindow();

    pthread_t computer_tid;
    bool draw_debug_hints = false;
    enum PlayerType players[2];
    players[0] = human;
    bool opponent_type_chosen = false;

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

        // First choose the type of opponent to play withs
        if (!opponent_type_chosen) {
            char *title = "Play Against";
            int title_size = 30;
            int width = MeasureText(title, title_size);
            V2 title_pos = {.x = BOARD_SIZE / 2 - width / 2, .y = BOARD_SIZE / 2 - title_size - 40};
            DrawText(title, title_pos.x, title_pos.y, title_size, WHITE);

            int margin = 10;
            int btn_width = 250;
            int btn_height = 50;
            V2 btn_init_pos = {.x = BOARD_SIZE / 2 - btn_width / 2, .y = title_pos.y + title_size + margin};
            int btn_text_size = 20;
            int text_padding = 15;
            char *opp1 = "Human";
            char *opp2 = "Computer";
            int size1 = MeasureText(opp1, btn_text_size);
            int size2 = MeasureText(opp2, btn_text_size);

            DrawRectangle(btn_init_pos.x, btn_init_pos.y, btn_width, btn_height, WHITE);
            DrawText(opp1, BOARD_SIZE / 2 - size1 / 2, btn_init_pos.y + text_padding, btn_text_size, COLOR_BLACK);
            int new_y = btn_init_pos.y + btn_height + margin;
            DrawRectangle(btn_init_pos.x, new_y, btn_width, btn_height, WHITE);
            DrawText(opp2, BOARD_SIZE / 2 - size2 / 2, new_y + text_padding, btn_text_size, COLOR_BLACK);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int x = GetMouseX();
                int y = GetMouseY();
                bool touched_inside =
                    x > btn_init_pos.x && x < btn_init_pos.x + btn_width &&
                    y > btn_init_pos.y && y < btn_init_pos.y + btn_height * 2 + margin;
                if (touched_inside) {
                    int y_offset = y - btn_init_pos.y;
                    players[1] = y_offset < btn_height ? human : computer;
                    opponent_type_chosen = true;
                }
            }

            EndDrawing();
            continue;
        }

        if (!board.checkmate) {
            enum PlayerType player = players[board.move_count % 2];
            if (player == human) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (board.promotion_pending)
                        handlePromotion(GetMouseX(), GetMouseY(), &board, pwin);
                    else
                        handleTouch(GetMouseX(), GetMouseY(), &board);
                }
            }
            else {
                if (!board.computer_thinking) {
                    board.computer_thinking = true;
                    pthread_create(&computer_tid, NULL, handleComputerTurn, (void *)&board);
                }
            }
        }

        if (IsKeyPressed(KEY_R) && !board.computer_thinking) {
            board = initBoard();
        }

        if (IsKeyPressed(KEY_E)) {
            float val = evaluateBoard(board);
            printf("value: %f\n", val);
        }

        if (IsKeyPressed(KEY_F)) {
            generateFEN(board);
        }

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

        if (board.checkmate) {
            char *text = "Checkmate!";
            int size = 50;
            int width = MeasureText(text, size);
            DrawText(text, BOARD_SIZE / 2 - width / 2, BOARD_SIZE / 2 - size / 2, size, RED);
        }

        bool drawn = board.draw_by_fifty_move;
        if (drawn) {
            char *text = "Draw!";
            int size = 50;
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
    fclose(log_file);
}