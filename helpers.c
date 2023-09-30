#include "helpers.h"

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