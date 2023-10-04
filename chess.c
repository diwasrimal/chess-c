#include "helpers.c"

int main(void)
{
    Color background = {.r = 31, .g = 31, .b = 40, .a = 255};
    Board board = initBoard();
    bool game_finished = false;

    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    while (!WindowShouldClose() && !game_finished) {
        BeginDrawing();
        ClearBackground(background);

        if (IsMouseButtonPressed(0)) {
            handleMove(GetMouseX(), GetMouseY(), &board);
        }

        if (IsKeyPressed(KEY_R)) {
            board = initBoard();
        }

        if (board.king_checked)
            board.checked_king->bg = RED;

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Cell c = board.cells[y][x];

                // char str[10];
                // sprintf(str, "%d %d", y, x);
                // DrawText(str, c.pos.x + 30, c.pos.y + 10, 20, PURPLE);

                const char *s = getPieceString(c);
                DrawRectangle(c.pos.x, c.pos.y, CELL_SIZE, CELL_SIZE, c.bg);

                if (c.is_dangerous[white]) {
                    DrawText("DW", c.pos.x + 40, c.pos.y + 10, 12, RED);
                }
                if (c.is_dangerous[black]) {
                    DrawText("DB", c.pos.x + 40, c.pos.y + 30, 12, RED);
                }

                if (c.blocks_check) {
                    DrawText("CBS", c.pos.x + 40, c.pos.y + 50, 10, BLACK);
                }

                if (c.opens_check) {
                    DrawText("PIN", c.pos.x + 40, c.pos.y + 70, 10, BLACK);
                }

                if (!s)
                    continue;
                DrawText(s, c.pos.x + 10, c.pos.y + 10, 20, (c.color == black) ? BLACK : GREEN);

            }
        }

        // if (board.checkmate) {
        //     // game_finished = true;
        //     const char *text = "Checkmate!";
        //     int fontSize = 50;
        //     int width = MeasureText(text, fontSize);
        //     DrawText(text, BOARD_SIZE / 2 - width / 2, BOARD_SIZE / 2 - fontSize / 2, fontSize, RED);
        //     EndDrawing();
        //     WaitTime(5);
        // }

        EndDrawing();
    }

    CloseWindow();
}