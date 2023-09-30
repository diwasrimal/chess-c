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

        for (int idx_y = 0; idx_y < 8; idx_y++) {
            for (int idx_x = 0; idx_x < 8; idx_x++) {
                Cell c = board.cells[idx_y][idx_x];
                const char *s = getPieceString(c);
                DrawRectangle(c.pos.x, c.pos.y, CELL_SIZE, CELL_SIZE, c.bg);
                if (!s) continue;
                DrawText(s, c.pos.x + 10, c.pos.y + 10, 20, (c.piece_color == black) ? BLACK : GREEN);
            }
        }

        EndDrawing();
    }

    CloseWindow();
}