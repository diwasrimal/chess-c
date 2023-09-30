#include <raylib.h>
#include <stdio.h>

#define CELL_SIZE 80
#define BOARD_PADDING 5
#define BOARD_SIZE (CELL_SIZE * 8)
#define WINDOW_SIZE (BOARD_SIZE + BOARD_PADDING * 2)

enum PieceColor {
    black,
    white,
    no_color,
};

enum PieceType {
    king,
    queen,
    bishop,
    knight,
    rook,
    pawn,
    no_type,
};

typedef struct {
    int x;
    int y;
} V2;

typedef struct {
    V2 pos;
    Color bg;
    enum PieceColor piece_color;
    enum PieceType piece_type;
} Cell;

typedef struct {
    Cell cells[8][8];
    Color bg_colors[2];
} Board;

Board initBoard(void);
char *getPieceString(Cell c);
V2 cellPosByIdx(int idx_x, int idx_y);
V2 cellIdxByPos(int pos_x, int pos_y);

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

Board initBoard(void)
{
    Board b;
    b.bg_colors[0] = RAYWHITE;
    b.bg_colors[1] = BROWN;

    // fill background and position of cells
    int count = 0;
    for (int idx_y = 0; idx_y < 8; idx_y++) {
        for (int idx_x = 0; idx_x < 8; idx_x++) {
            b.cells[idx_y][idx_x].pos = cellPosByIdx(idx_x, idx_y);
            b.cells[idx_y][idx_x].bg = b.bg_colors[count % 2];
            count++;
        }
        count++;
    }

    enum PieceType main_row_black[] = {rook, knight, bishop, king, queen, bishop, knight, rook};
    enum PieceType main_row_white[] = {rook, knight, bishop, queen, king, bishop, knight, rook};

    // Black pieces (top)
    for (int i = 0; i < 8; i++) {
        b.cells[0][i].piece_color = black;
        b.cells[0][i].piece_type = main_row_black[i];
    }
    for (int i = 0; i < 8; i++) {
        b.cells[1][i].piece_color = black;
        b.cells[1][i].piece_type = pawn;
    }

    // Empty pieces
    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            b.cells[i][j].piece_color = no_color;
            b.cells[i][j].piece_type = no_type;
        }
    }

    // White pieces (bottom)
    for (int i = 0; i < 8; i++) {
        b.cells[6][i].piece_color = white;
        b.cells[6][i].piece_type = pawn;
    }
    for (int i = 0; i < 8; i++) {
        b.cells[7][i].piece_color = white;
        b.cells[7][i].piece_type = main_row_white[i];
    }

    return b;
}

// Returns the drawing position
V2 cellPosByIdx(int idx_x, int idx_y)
{
    V2 vec;
    vec.x = BOARD_PADDING + (idx_x * CELL_SIZE);
    vec.y = BOARD_PADDING + (idx_y * CELL_SIZE);
    return vec;
}

V2 cellIdxByPos(int pos_x, int pos_y)
{
    V2 vec;
    vec.x = (pos_x - BOARD_PADDING) / CELL_SIZE;
    vec.y = (pos_y - BOARD_PADDING) / CELL_SIZE;
    return vec;
}


char *getPieceString(Cell c)
{
    switch (c.piece_type) {
    case rook:
        return "R";
    case knight:
        return "Kn";
    case bishop:
        return "B";
    case king:
        return "K";
    case queen:
        return "Q";
    case pawn:
        return "P";
    case no_type:
        return NULL;
    }
}