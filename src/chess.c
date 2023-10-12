#include "helpers.c"

int main(void)
{
    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    Color background = {.r = 31, .g = 31, .b = 40, .a = 255};
    Board board = initBoard();
    PromotionWindow pwin = initPromotionWindow();
    bool draw_debug_hints = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(background);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !board.checkmate) {
            if (board.promotion_pending)
                handlePromotion(GetMouseX(), GetMouseY(), &board, pwin);
            else
                handleTouch(GetMouseX(), GetMouseY(), &board);
        }

        if (IsKeyPressed(KEY_R)) {
            board = initBoard();
        }

        if (board.king_checked)
            board.checked_king->bg = RED;

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Cell c = board.cells[y][x];
                const char *s = getPieceTypeString(c.piece);
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

                if (!s)
                    continue;
                DrawText(s, c.pos.x + 10, c.pos.y + 10, 20, (c.piece.color == black) ? BLACK : GREEN);
            }
        }

        if (board.promotion_pending) {
            enum PieceColor promoting_color = board.promoting_cell->piece.color;
            DrawRectangle(pwin.pos.x, pwin.pos.y, pwin.width, pwin.height, background);
            DrawText(pwin.text, pwin.pos.x + pwin.width / 2 - pwin.text_width / 2, pwin.pos.y + pwin.padding, pwin.text_height, RAYWHITE);

            for (int i = 0; i < 4; i++) {
                int posx = pwin.first_cell_pos.x + i * (CELL_SIZE + pwin.cell_margin);
                DrawRectangle(posx, pwin.first_cell_pos.y, CELL_SIZE, CELL_SIZE, RAYWHITE);
                Piece p = { .type = pwin.promotables[i], .color = promoting_color};
                DrawText(getPieceTypeString(p), posx + 10, pwin.first_cell_pos.y + 10, 20, (promoting_color == black) ? BLACK : GREEN);
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

    CloseWindow();
}