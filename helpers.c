#include "helpers.h"

Board initBoard(void)
{
    Board b;
    b.bg_colors[0] = RAYWHITE;
    b.bg_colors[1] = BROWN;

    // fill background and position of cells
    int count = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b.cells[y][x].pos = cellPosByIdx(x, y);
            b.cells[y][x].bg = b.bg_colors[count % 2];
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
V2 cellPosByIdx(int x, int y)
{
    V2 vec;
    vec.x = BOARD_PADDING + (x * CELL_SIZE);
    vec.y = BOARD_PADDING + (y * CELL_SIZE);
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