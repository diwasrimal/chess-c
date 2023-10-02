#include "helpers.h"

Board initBoard(void)
{
    Board b;

    resetCellBackgrounds(&b);
    b.move_pending = false;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b.cells[y][x].pos = cellPosByIdx(x, y);
            b.cells[y][x].is_valid = false;
        }
    }

    // First turn
    b.turn = white;

    enum PieceType main_row_black[] = {rook, knight, bishop, king, queen, bishop, knight, rook};
    enum PieceType main_row_white[] = {rook, knight, bishop, queen, king, bishop, knight, rook};

    // Black pieces (top)
    for (int i = 0; i < 8; i++) {
        b.cells[0][i].color = black;
        b.cells[0][i].type = main_row_black[i];
    }
    for (int i = 0; i < 8; i++) {
        b.cells[1][i].color = black;
        b.cells[1][i].type = pawn;
    }

    // Empty pieces
    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            b.cells[i][j].color = no_color;
            b.cells[i][j].type = no_type;
        }
    }

    // White pieces (bottom)
    for (int i = 0; i < 8; i++) {
        b.cells[6][i].color = white;
        b.cells[6][i].type = pawn;
    }
    for (int i = 0; i < 8; i++) {
        b.cells[7][i].color = white;
        b.cells[7][i].type = main_row_white[i];
    }

    // Record dangerous cells for opponent
    recordDangerousCells(&b);

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
    switch (c.type) {
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

void handleMove(int mouse_x, int mouse_y, Board *b)
{
    resetCellBackgrounds(b);
    V2 idx = cellIdxByPos(mouse_x, mouse_y);
    Cell *touched = &(b->cells[idx.y][idx.x]);

    if (b->move_pending && touched->is_valid) {
        touched->type = b->active_cell->type;
        touched->color = b->active_cell->color;
        b->active_cell->type = no_type;
        b->active_cell->color = no_color;
        b->move_pending = false;
        b->turn = (b->turn == black) ? white : black;
        recordDangerousCells(b);
        recordCheckToKing(b);
        return;
    }

    if (touched->color != b->turn) {
        fprintf(stderr, "Not your turn\n");
        return;
    }

    // printf("No move pendign\n");

    // Ignore touches on empty cells
    if (touched->type == no_type)
        return;

    touched->bg = YELLOW;
    b->active_cell = touched;

    // Reset possible and valid moves
    b->move_pending = false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            b->cells[i][j].is_valid = false;
        }
    }

    switch (touched->type) {
    case pawn:
        fillPossibleMovesPawn(idx.x, idx.y, b);
        break;
    case rook:
    case bishop:
        fillPossibleMovesContinuous(idx.x, idx.y, touched->type, b);
        break;
    case queen:
        fillPossibleMovesContinuous(idx.x, idx.y, rook, b);
        fillPossibleMovesContinuous(idx.x, idx.y, bishop, b);
        break;
    case knight:
        fillPossibleMovesKnight(idx.x, idx.y, b);
        break;
    case king:
        fillPossibleMovesKing(idx.x, idx.y, b);
        break;
    default:
        fprintf(stderr, "Not implemented!\n");
    }

    // Color valid moves and set pending to true
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (b->cells[i][j].is_valid) {
                b->move_pending = true;
                bool opposing =
                    (b->cells[i][j].color != touched->color) &&
                    (b->cells[i][j].type != no_type);
                b->cells[i][j].bg = opposing ? RED : BLUE;
            }
        }
    }
}

bool validCellIdx(int x, int y)
{
    return (0 <= x && x < 8) && (0 <= y && y < 8);
}

void fillPossibleMovesPawn(int x, int y, Board *b)
{
    Cell touched = b->cells[y][x];

    // Direction of move: black goes down, white goes up
    int dir = (touched.color == black) ? 1 : -1;
    int starting_pos = (touched.color == black) ? 1 : 6;
    bool in_starting_position = y == starting_pos;

    // Straight move (should be empty)
    int move_limit = in_starting_position ? 2 : 1;
    for (int dy = 1; dy <= move_limit; dy++) {
        int j = y + dy * dir;
        if (!validCellIdx(x, j))
            break;
        bool empty = b->cells[j][x].type == no_type;
        if (!empty)
            break;
        b->cells[j][x].is_valid = true;
    }

    // Diagonal moves (only captures)
    int j = y + dir;
    int xleft = x - 1;
    int xright = x + 1;

    b->cells[j][xleft].is_valid =
        validCellIdx(xleft, j) &&
        b->cells[j][xleft].type != no_type &&       // mustn't be empty
        b->cells[j][xleft].color != touched.color;  // must be of different color

    b->cells[j][xright].is_valid =
        validCellIdx(xright, j) &&
        b->cells[j][xright].type != no_type &&
        b->cells[j][xright].color != touched.color;
}


void fillPossibleMovesContinuous(int x, int y, enum PieceType t, Board *b)
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

    enum PieceColor curr_color = b->cells[y][x].color;

    for (int l = 0; l < 2; l++) {
        for (int m = 0; m < 2; m++) {
            V2 vec = vectors[l][m];
            int i = x + vec.x;
            int j = y + vec.y;

            while (validCellIdx(i, j)) {
                bool empty = b->cells[j][i].type == no_type;
                bool ours = b->cells[j][i].color == curr_color;
                bool opponent = !empty && !ours;
                if (ours) break;
                if (opponent) {
                    b->cells[j][i].is_valid = true;
                    break;
                }
                b->cells[j][i].is_valid = true;
                i += vec.x;
                j += vec.y;
            }
        }
    }
}

void fillPossibleMovesKnight(int x, int y, Board *b)
{
    enum PieceColor curr_color = b->cells[y][x].color;

    int dy[8] = {2, 2, -2, -2, 1, 1, -1, -1};
    int dx[8] = {-1, 1, -1, 1, -2, 2, -2, 2};
    for (int k = 0; k < 8; k++) {
        int i = x + dx[k];
        int j = y + dy[k];
        if (!validCellIdx(i, j)) continue;
        bool ours = b->cells[j][i].color == curr_color;
        if (!ours)
            b->cells[j][i].is_valid = true;
    }
}

void fillPossibleMovesKing(int x, int y, Board *b)
{
    Cell touched = b->cells[y][x];

    for (int j = y - 1; j <= y + 1; j++) {
        for (int i = x - 1; i <= x + 1; i++) {
            if (i == x && j == y) continue;
            if (!validCellIdx(i, j)) continue;
            if (b->cells[j][i].is_dangerous[touched.color]) continue;
            bool ours = b->cells[j][i].color == touched.color;
            if (!ours) {
                b->cells[j][i].is_valid = true;
            }
        }
    }
}

// Record cells that will become dangerous to opponent
void recordDangerousCells(Board *b)
{
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].is_dangerous[black] = false;
            b->cells[y][x].is_dangerous[white] = false;
        }
    }

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell c = b->cells[y][x];

            // Handle pawns differently
            if (c.type == pawn) {
                recordDangerousCellsByPawn(x, y, b);
                continue;
            }

            // Simulate touching the piece using a temporary board
            Board tmp = *b;
            tmp.turn = c.color;
            tmp.move_pending = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    tmp.cells[i][j].is_valid = false;

            handleMove(c.pos.x, c.pos.y, &tmp);

            // tmp->valid moves will become dangerous for opponent
            enum PieceColor opposing = c.color == black ? white : black;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (tmp.cells[i][j].is_valid) {
                        printf("%d %d valid for %d %d\n", i, j, y, x);
                        b->cells[i][j].is_dangerous[opposing] = true;
                    }
                }
            }
        }
    }
}

// Record cells that will become dangerous to opponent after we move our pawn's
void recordDangerousCellsByPawn(int x, int y, Board *b)
{
    // Pawn captures only diagonals, thus threatens only diagonals
    // Threatens both empty and occupied cells
    Cell touched = b->cells[y][x];
    int dir = (touched.color == black) ? 1 : -1;
    int j = y + dir;
    int xl = x - 1;
    int xr = x + 1;

    enum PieceColor dangerous_for = touched.color == black ? white : black;

    b->cells[j][xl].is_dangerous[dangerous_for] =
        validCellIdx(xl, j) &&
        b->cells[j][xl].color != touched.color;

    b->cells[j][xr].is_dangerous[dangerous_for] =
        validCellIdx(xr, j) &&
        b->cells[j][xr].color != touched.color;
}

// Finds whether opponent's king has been checked
void recordCheckToKing(Board *b)
{
    b->king_checked[white] = false;
    b->king_checked[black] = false;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Cell c = b->cells[i][j];
            enum PieceColor color = c.color;
            bool cell_dangerous = c.is_dangerous[color];
            if (cell_dangerous && c.type == king) {
                b->king_checked[color] = true;
                b->cells[i][j].bg = RED;
            }
        }
    }
}