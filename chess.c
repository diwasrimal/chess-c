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

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Cell c = board.cells[y][x];
                const char *s = getPieceString(c);
                DrawRectangle(c.pos.x, c.pos.y, CELL_SIZE, CELL_SIZE, c.bg);
                if (!s)
                    continue;
                DrawText(s, c.pos.x + 10, c.pos.y + 10, 20, (c.piece_color == black) ? BLACK : GREEN);
            }
        }

        EndDrawing();
    }

    CloseWindow();
}