#include "helpers.h"

Board initBoard(void)
{
    Board b;

    // fill background and position of cells
    resetCellBackgrounds(&b);
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            b.cells[y][x].pos = cellPosByIdx(x, y);

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

MoveHandler initMoveHandler(void)
{
    MoveHandler handler;
    handler.move_pending = false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            handler.possible[i][j] = false;
            handler.valid[i][j] = false;
        }
    }
    return handler;
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

void resetCellBackgrounds(Board *b)
{
    Color colors[] = {RAYWHITE, BROWN};

    int count = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].bg = colors[count % 2];
            count++;
        }
        count++;
    }
}

void handleMove(int mouse_x, int mouse_y, Board *b, MoveHandler *h)
{
    resetCellBackgrounds(b);
    V2 idx = cellIdxByPos(mouse_x, mouse_y);
    Cell *touched = &(b->cells[idx.y][idx.x]);

    if (h->move_pending && h->valid[idx.y][idx.x]) {
        printf("Move pending\n");
        touched->piece_type = h->active_cell->piece_type;
        touched->piece_color = h->active_cell->piece_color;
        h->active_cell->piece_type = no_type;
        h->active_cell->piece_color = no_color;
        h->move_pending = false;
        return;
    }

    printf("No move pendign\n");

    // Ignore touches on empty cells
    if (touched->piece_type == no_type)
        return;

    touched->bg = YELLOW;
    h->active_cell = touched;

    // Reset possible and valid moves
    h->move_pending = false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            h->possible[i][j] = false;
            h->valid[i][j] = false;
        }
    }

    switch (touched->piece_type) {
    case pawn:
        fillPossibleMovesPawn(idx.x, idx.y, (touched->piece_color == black), h, b);
        break;
    case rook:
    case bishop:
        fillPossibleMovesContinuous(idx.x, idx.y, touched->piece_type, h, b);
        break;
    case queen:
        fillPossibleMovesContinuous(idx.x, idx.y, rook, h, b);
        fillPossibleMovesContinuous(idx.x, idx.y, bishop, h, b);
        break;
    case knight:
        fillPossibleMovesKnight(idx.x, idx.y, h, b);
    default:
        fprintf(stderr, "Not implemented!\n");
    }

    // Color valid moves and set pending to true
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (h->valid[i][j]) {
                h->move_pending = true;
                bool opposing =
                    (b->cells[i][j].piece_color != touched->piece_color) &&
                    (b->cells[i][j].piece_type != no_type);
                b->cells[i][j].bg = opposing ? RED : BLUE;
            }
        }
    }
}

bool validCellIdx(int x, int y)
{
    return (0 <= x && x < 8) && (0 <= y && y < 8);
}

void fillPossibleMovesPawn(int x, int y, bool isBlack, MoveHandler *h, const Board *b)
{
    // Direction of move: black goes down, white goes up
    int dir = isBlack ? 1 : -1;
    bool in_starting_position = (y == (isBlack ? 1 : 6));
    enum PieceColor curr_color = b->cells[y][x].piece_color;


    // Straight move (should be empty)
    int move_limit = in_starting_position ? 2 : 1;
    for (int dy = 1; dy <= move_limit; dy++) {
        int j = y + dy * dir;
        if (!validCellIdx(x, j))
            break;
        bool empty = b->cells[j][x].piece_type == no_type;
        if (!empty)
            break;
        h->valid[j][x] = true;
    }

    // Diagonal moves (only captures)
    int j = y + dir;
    int xleft = x - 1;
    int xright = x + 1;

    h->valid[j][xleft] =
        validCellIdx(xleft, j) &&
        b->cells[j][xleft].piece_type != no_type &&
        b->cells[j][xleft].piece_color != curr_color;

    h->valid[j][xright] =
        validCellIdx(xright, j) &&
        b->cells[j][xright].piece_type != no_type &&
        b->cells[j][xright].piece_color != curr_color;
}


void fillPossibleMovesContinuous(int x, int y, enum PieceType t, MoveHandler *h, const Board *b)
{
    V2 vectors[2][2];
    switch (t) {
    case rook:
        vectors[0][0] = (V2){.y = -1, .x = 0};  // up
        vectors[0][1] = (V2){.y = 1, .x = 0};   // down
        vectors[1][0] = (V2){.y = 0, .x = -1};  // left
        vectors[1][1] = (V2){.y = 0, .x = 1};   // right
        break;
    case bishop:
        vectors[0][0] = (V2){.y = -1, .x = -1};  // top left
        vectors[0][1] = (V2){.y = -1, .x = 1};   // top right
        vectors[1][0] = (V2){.y = 1, .x = -1};   // bot left
        vectors[1][1] = (V2){.y = 1, .x = 1};    // bot right
        break;
    default:
        fprintf(stderr, "Not implemented continuous move for type: %d\n", t);
    }

    enum PieceColor curr_color = b->cells[y][x].piece_color;

    for (int l = 0; l < 2; l++) {
        for (int m = 0; m < 2; m++) {
            V2 vec = vectors[l][m];
            int i = x + vec.x;
            int j = y + vec.y;

            while (validCellIdx(i, j)) {
                bool empty = b->cells[j][i].piece_type == no_type;
                bool ours = b->cells[j][i].piece_color == curr_color;
                bool opponent = !empty && !ours;
                if (ours) break;
                if (opponent) {
                    h->valid[j][i] = true;
                    break;
                }
                h->valid[j][i] = true;
                i += vec.x;
                j += vec.y;
            }
        }
    }
}

void fillPossibleMovesKnight(int x, int y, MoveHandler *h, const Board *b)
{
    enum PieceColor curr_color = b->cells[y][x].piece_color;

    int dy[8] = {2, 2, -2, -2, 1, 1, -1, -1};
    int dx[8] = {-1, 1, -1, 1, -2, 2, -2, 2};
    for (int k = 0; k < 8; k++) {
        int i = x + dx[k];
        int j = y + dy[k];
        if (!validCellIdx(i, j)) continue;
        bool ours = b->cells[j][i].piece_color == curr_color;
        if (!ours)
            h->valid[j][i] = true;
    }
}
