#include <raylib.h>
#include <stdio.h>

#define CELL_SIZE 80
#define BOARD_PADDING 5
#define BOARD_SIZE (CELL_SIZE * 8)
#define WINDOW_SIZE (BOARD_SIZE + BOARD_PADDING * 2)

int main(void)
{
    Color background = {.r = 31, .g = 31, .b = 40, .a = 255};
    Color checkers[] = {RAYWHITE, BROWN};

    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(background);

        int round = 0;
        for (int y = BOARD_PADDING; y < BOARD_SIZE; y += CELL_SIZE) {
            for (int x = BOARD_PADDING; x < BOARD_SIZE; x += CELL_SIZE) {
                DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, checkers[round % 2]);
                round++;
            }
            round++;
        }
        EndDrawing();
    }

    CloseWindow();
}