#include "helpers.c"

int main(void)
{
    Color background = {.r = 31, .g = 31, .b = 40, .a = 255};
    Board board = initBoard();

    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(background);

        if (IsMouseButtonPressed(0)) {
            handleMove(GetMouseX(), GetMouseY(), &board);
        }

        if (IsKeyPressed(KEY_R)) {
            board = initBoard();
        }

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Cell c = board.cells[y][x];
                const char *s = getPieceString(c);
                DrawRectangle(c.pos.x, c.pos.y, CELL_SIZE, CELL_SIZE, c.bg);
                
                // char str[10];
                // sprintf(str, "%d %d", y, x);
                // DrawText(str, c.pos.x + 30, c.pos.y + 10, 20, PURPLE);

                if (board.cells[y][x].is_dangerous[white])
                    DrawText("DW", c.pos.x + 40, c.pos.y + 10, 20, RED);
                if (board.cells[y][x].is_dangerous[black])
                    DrawText("DB", c.pos.x + 40, c.pos.y + 40, 20, RED);

                if (!s)
                    continue;
                DrawText(s, c.pos.x + 10, c.pos.y + 10, 20, (c.color == black) ? BLACK : GREEN);
            }
        }

        EndDrawing();
    }

    CloseWindow();
}