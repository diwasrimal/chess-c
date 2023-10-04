#include "helpers.h"

Board initBoard(void)
{
    Board b;

    resetCellBackgrounds(&b);
    b.turn = white;
    b.move_pending = false;
    b.checkmate = false;
    b.king_checked = false;
    b.filter_nonblocking_cells = true;
    b.checked_king = NULL;
    b.active_cell = NULL;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b.cells[y][x].pos = cellPosByIdx(x, y);
            b.cells[y][x].in_range = false;
            b.cells[y][x].is_movable = false;
            b.cells[y][x].blocks_check = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    b.cells[y][x].check_blocking_cells[i][j] = false;
        }
    }

    enum PieceType main_row[] = {rook, knight, bishop, queen, king, bishop, knight, rook};

    // Black pieces (top)
    for (int i = 0; i < 8; i++) {
        b.cells[0][i].color = black;
        b.cells[0][i].type = main_row[i];
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
        b.cells[7][i].color = white;
        b.cells[7][i].type = main_row[i];
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
    // printf("\nhandleMove()\n");
    resetCellBackgrounds(b);
    V2 idx = cellIdxByPos(mouse_x, mouse_y);
    Cell *touched = &(b->cells[idx.y][idx.x]);

    if (b->move_pending) {
        if (touched->is_movable) {
            printf("b->move_pending && touched->is_movable\n");
            touched->type = b->active_cell->type;
            touched->color = b->active_cell->color;
            b->active_cell->type = no_type;
            b->active_cell->color = no_color;
            b->move_pending = false;
            b->turn = (b->turn == black) ? white : black;
            recordDangerousCells(b);
            recordCheck(b);
            return;
        }
        else {
            printf("b->move_pendng && !touched->is_movable\n");
            b->move_pending = false;
        }
    }

    if (touched->color != b->turn) {
        printf("touched->color != b->turn\n");
        return;
    }

    // Ignore touches on empty cells
    if (emptyCell(*touched))
        return;

    touched->bg = YELLOW;
    b->active_cell = touched;

    // Reset cells in range and cells that are movable
    b->move_pending = false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            b->cells[i][j].in_range = false;
            b->cells[i][j].is_movable = false;
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

    // Filter moves
    // printf("Filtration:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            Cell *cell = &(b->cells[i][j]);
            if (!cell->in_range)
                continue;

            // printf("\t%d %d:\n", i, j);
            if (cell->color == touched->color) {
                // printf("\t\tcell->color == touched->color\n");
                continue;
            }

            // Filter cells dangerous if king is moving
            if (touched->type == king && cell->is_dangerous[touched->color]) {
                // printf("\t\ttouched->type == king && cell->is_dangerous[touched->color]\n");
                continue;
            }

            // Filter cells that don't block check when some piece moves there
            if (b->king_checked && b->filter_nonblocking_cells && !touched->check_blocking_cells[i][j]) {
                // printf("\t\tb->king_checked && b->filter_nonblocking_cells && !touched->blocking_cells[i][j]\n");
                continue;
            }

            cell->is_movable = true;
            b->move_pending = true;
            cell->bg = (cell->color == no_color) ? BLUE : RED;
        }
    }
}

bool validCellIdx(int x, int y)
{
    return (0 <= x && x < 8) && (0 <= y && y < 8);
}

bool emptyCell(Cell c)
{
    return c.type == no_type && c.color == no_color;
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
        if (!validCellIdx(x, j) || !emptyCell(b->cells[j][x]))
            break;
        b->cells[j][x].in_range = true;
    }

    // Diagonal moves (only captures)
    int j = y + dir;
    int xl = x - 1;
    int xr = x + 1;

    if (validCellIdx(xl, j) && !emptyCell(b->cells[j][xl]))
        b->cells[j][xl].in_range = true;

    if (validCellIdx(xr, j) && !emptyCell(b->cells[j][xr]))
        b->cells[j][xr].in_range = true;
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

    for (int l = 0; l < 2; l++) {
        for (int m = 0; m < 2; m++) {
            V2 vec = vectors[l][m];
            int i = x + vec.x;
            int j = y + vec.y;

            while (validCellIdx(i, j)) {
                if (!emptyCell(b->cells[j][i])) {
                    b->cells[j][i].in_range = true;
                    break;
                }
                b->cells[j][i].in_range = true;
                i += vec.x;
                j += vec.y;
            }
        }
    }
}

void fillPossibleMovesKnight(int x, int y, Board *b)
{
    int dy[8] = {2, 2, -2, -2, 1, 1, -1, -1};
    int dx[8] = {-1, 1, -1, 1, -2, 2, -2, 2};
    for (int k = 0; k < 8; k++) {
        int i = x + dx[k];
        int j = y + dy[k];
        if (!validCellIdx(i, j)) continue;
        b->cells[j][i].in_range = true;
    }
}

void fillPossibleMovesKing(int x, int y, Board *b)
{
    for (int j = y - 1; j <= y + 1; j++) {
        for (int i = x - 1; i <= x + 1; i++) {
            if ((i == x && j == y) || !validCellIdx(i, j))
               continue;
            b->cells[j][i].in_range = true;
        }
    }
}

// Record cells that will become dangerous to opponent
void recordDangerousCells(Board *b)
{
    printf("recordDangerousCells()\n");
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].is_dangerous[black] = false;
            b->cells[y][x].is_dangerous[white] = false;
            // printf("%d%d ", y, x);
        }
        // printf("\n");
    }

    // printf("\tPieces in board");
    // for (int i =0 ; i < 8; i++) {
    //     for (int j = 0; j <8; j++) {
    //         printf("\t\t%d %d:\n", i, j);
    //         printf("\t\t\ttype: %d, color: %d\n", b->cells[i][j].type, b->cells[i][j].color);
    //     }
    // }


    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            // printf("\t%d %d\n", y, x);
            Cell c = b->cells[y][x];
            if (emptyCell(c))
                continue;

            enum PieceColor opposing = (c.color == black) ? white : black;
            // printf("Before touch b:\n");
            // for (int i = 0; i < 8; i++) {
            //     printf("\t");
            //     for (int j = 0; j < 8; j++) {
            //         printf("%d ", b->cells[i][j].is_dangerous[opposing]);
            //     }
                // printf("\n");
            // }

            // Handle pawns differently
            if (c.type == pawn) {
                // printf("c.type == pawn\n");
                recordDangerousCellsByPawn(x, y, b);
                continue;
            }

            // Simulate touching the piece using a temporary board
            // Find cells that are in range, those cells will be dangerous
            // for opponent to enter
            Board tmp = *b;
            tmp.turn = c.color;
            tmp.move_pending = false;

            handleMove(c.pos.x, c.pos.y, &tmp);

            // printf("After touch:\n");
            for (int i = 0; i < 8; i++) {
                // printf("\t\t");
                for (int j = 0; j < 8; j++) {
                    // printf("%d ", tmp.cells[i][j].in_range);
                    if (tmp.cells[i][j].in_range)
                        b->cells[i][j].is_dangerous[opposing] = true;
                }
                // printf("\n");
            }
        }
    }
}

// Record cells that will become dangerous to opponent after we move our pawn
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
    if (validCellIdx(xl, j))
        b->cells[j][xl].is_dangerous[dangerous_for] = true;
    if (validCellIdx(xr, j))
        b->cells[j][xr].is_dangerous[dangerous_for] = true;
}

// Finds whether king is checked, and finds cells that can block the check
void recordCheck(Board *b)
{
    printf("recordCheck()\n");

    enum PieceColor king_color = b->turn;
    V2 king_idx;
    b->king_checked = false;
    b->filter_nonblocking_cells = false;
    b->checked_king = NULL;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].blocks_check = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    b->cells[y][x].check_blocking_cells[i][j] = false;
        }
    }

    // Find if king checked
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Cell c = b->cells[i][j];
            if (c.type == king && c.color == king_color && c.is_dangerous[king_color]) {
                printf("c.type == king && c.color == king_color && c.is_dangerous[king_color]\n");
                b->king_checked = true;
                b->checked_king = &(b->cells[i][j]);
                b->filter_nonblocking_cells = true;
                king_idx.y = i;
                king_idx.x = j;
            }
        }
    }

    if (!b->king_checked) {
        printf("!b->king_checked\n");
        return;
    }

    // Find cells that will block the check
    // Simulate moving all pieces to each of their movable cells
    // If that makes the king safe, that moved cell blocks the check
    bool can_be_blocked = false;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            Cell src = b->cells[y][x];
            if (emptyCell(src))
                continue;

            if (src.color != king_color)
                continue;

            // Collect movable moves (dont filter non blocking moves)
            // Otherwise everything may get filtered in first try
            printf("src: %d %d:\n", y, x);
            Board tmp1 = *b;
            tmp1.move_pending = false;
            tmp1.filter_nonblocking_cells = false;
            handleMove(src.pos.x, src.pos.y, &tmp1);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {

                    Cell dest = tmp1.cells[i][j];
                    if (!dest.is_movable)
                        continue;

                    printf("\tdest: %d %d:\n", i, j);

                    Board tmp2 = tmp1;
                    tmp2.cells[i][j].type = src.type;
                    tmp2.cells[i][j].color = src.color;
                    tmp2.cells[y][x].type = no_type;
                    tmp2.cells[y][x].color = no_color;

                    // for (int l = 0; l < 8; l++) {
                    //     for (int m = 0; m < 8; m++) {
                    //         printf("\t\ttype: %s, color: %s, idx: %d %d\n", getPieceString(tmp2.cells[l][m]), tmp2.cells[l][m].color == white ? "white" : "black", l, m);
                    //     }
                    //     printf("\n");
                    // }

                    recordDangerousCells(&tmp2);

                    // printf("\tDangerous:\n");
                    // for (int l = 0; l < 8; l++) {
                    //     printf("\t\t");
                    //     for (int m = 0; m < 8; m++) {
                    //         printf("%d ", tmp2.cells[l][m].is_dangerous[king_color]);
                    //     }
                    //     printf("\n");
                    // }

                    // If king is safe now, src blocks the check
                    // If src was king, king is at (i, j) now
                    V2 king_at = (src.type == king) ? (V2){.y = i, .x = j} : king_idx;
                    bool king_safe = !tmp2.cells[king_at.y][king_at.x].is_dangerous[king_color];
                    printf("\tKing safe: %d\n", king_safe);
                    if (king_safe) {
                        can_be_blocked = true;
                        b->cells[y][x].blocks_check = true;
                        b->cells[y][x].check_blocking_cells[i][j] = true;
                    }
                }
            }
        }
    }

    if (!can_be_blocked) {
        printf("Checkmate\n");
        b->checkmate = true;
    }
}