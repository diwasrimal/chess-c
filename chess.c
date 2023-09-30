#include <raylib.h>
#include <stdio.h>

#define CELL_SIZE 80
#define BOARD_PADDING 5
#define BOARD_SIZE (CELL_SIZE * 8)
#define WINDOW_SIZE (BOARD_SIZE + BOARD_PADDING * 2)

typedef struct {
    int pos_x;
    int pos_y;
    Color color;
} Cell;

typedef struct {
    Cell cells[8][8];
    Color colors[2];
    int rows;
    int cols;
} Board;

Board initBoard(void);
void updateCellPosition(Cell *c, int idx_y, int idx_x);

int main(void)
{
    Color background = {.r = 31, .g = 31, .b = 40, .a = 255};
    Board board = initBoard();

    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(background);

        for (int idx_y = 0; idx_y < board.rows; idx_y++) {
            for (int idx_x = 0; idx_x < board.cols; idx_x++) {
                Cell c = board.cells[idx_y][idx_x];
                DrawRectangle(c.pos_x, c.pos_y, CELL_SIZE, CELL_SIZE, c.color);
            }
        }

        EndDrawing();
    }

    CloseWindow();
}

Board initBoard(void)
{
    Board b;
    b.colors[0] = RAYWHITE;
    b.colors[1] = BROWN;
    b.rows = 8;
    b.cols = 8;

    int count = 0;
    for (int idx_y = 0; idx_y < b.rows; idx_y++) {
        for (int idx_x = 0; idx_x < b.cols; idx_x++) {
            updateCellPosition(&b.cells[idx_y][idx_x], idx_y, idx_x);
            b.cells[idx_y][idx_x].color = b.colors[count % 2];
            count++;
        }
        count++;
    }

    return b;
}

void updateCellPosition(Cell *c, int idx_y, int idx_x)
{
    c->pos_x = BOARD_PADDING + (idx_x * CELL_SIZE);
    c->pos_y = BOARD_PADDING + (idx_y * CELL_SIZE);
}