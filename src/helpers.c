#include "helpers.h"

Board initBoard(void)
{
    Board b;

    resetCellBackgrounds(&b);
    b.turn = white;
    b.move_pending = false;
    b.promotion_pending = false;
    b.checkmate = false;
    b.king_checked = false;
    b.filter_nonblocking_cells = true;
    b.filter_check_opening = true;
    b.king_moved[white] = false;
    b.king_moved[black] = false;
    b.left_rook_moved[white] = false;
    b.left_rook_moved[black] = false;
    b.right_rook_moved[white] = false;
    b.right_rook_moved[black] = false;
    b.checked_king = NULL;
    b.active_cell = NULL;
    b.promoting_cell = NULL;
    b.left_castling_cell[black] = NULL;
    b.left_castling_cell[white] = NULL;
    b.right_castling_cell[black] = NULL;
    b.right_castling_cell[white] = NULL;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b.cells[y][x].pos = cellPosByIdx(x, y);
            b.cells[y][x].in_range = false;
            b.cells[y][x].is_movable = false;
            b.cells[y][x].is_dangerous[black] = false;  // TODO: maybe prevent segfault if pos of black/white change in enum `ColorType`
            b.cells[y][x].is_dangerous[white] = false;
            b.cells[y][x].blocks_check = false;
            b.cells[y][x].opens_check = false;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    b.cells[y][x].check_blocking_cells[i][j] = false;
                    b.cells[y][x].check_opening_cells[i][j] = false;
                }
            }
        }
    }

    enum PieceType main_row[] = {rook, knight, bishop, queen, king, bishop, knight, rook};

    // Black pieces (top)
    for (int i = 0; i < 8; i++) {
        b.cells[0][i].piece.color = black;
        b.cells[0][i].piece.type = main_row[i];
        b.cells[1][i].piece.color = black;
        b.cells[1][i].piece.type = pawn;
    }

    // Empty pieces
    Piece empty_piece = {.color = no_color, .type = no_type};
    for (int i = 2; i < 6; i++)
        for (int j = 0; j < 8; j++)
            b.cells[i][j].piece = empty_piece;

    // White pieces (bottom)
    for (int i = 0; i < 8; i++) {
        b.cells[6][i].piece.color = white;
        b.cells[6][i].piece.type = pawn;
        b.cells[7][i].piece.color = white;
        b.cells[7][i].piece.type = main_row[i];
    }

    // Record dangerous cells for opponent
    recordDangerousCells(&b);

    return b;
}

PromotionWindow initPromotionWindow(void)
{
    PromotionWindow pwin;
    pwin.promotables[0] = queen;
    pwin.promotables[1] = rook;
    pwin.promotables[2] = knight;
    pwin.promotables[3] = bishop;

    pwin.padding = 10;
    pwin.cell_margin = 5;

    pwin.text = "Promote To";
    pwin.text_height = 30;
    pwin.text_width = MeasureText(pwin.text, pwin.text_height);

    pwin.height = CELL_SIZE + pwin.padding * 3 + pwin.text_height;
    pwin.width = CELL_SIZE * 4 + pwin.cell_margin * 3 + pwin.padding * 2;

    pwin.pos.x = BOARD_SIZE / 2 - pwin.width / 2;
    pwin.pos.y = BOARD_SIZE / 2 - pwin.height / 2;
    pwin.first_cell_pos.x = pwin.pos.x + pwin.padding;
    pwin.first_cell_pos.y = pwin.pos.y + pwin.padding * 2 + pwin.text_height;

    return pwin;
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


char *getPieceTypeString(Piece p)
{
    switch (p.type) {
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
    return NULL;
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

            enum PieceColor active_color = b->active_cell->piece.color;
            enum PieceType active_type = b->active_cell->piece.type;
            printf("b->move_pending && touched->is_movable\n");


            // Record movement of castling pieces
            int moved_x = cellIdxByPos(b->active_cell->pos.x, b->active_cell->pos.y).x;

            printf("Active cell: %s\n", getPieceTypeString(b->active_cell->piece));
            if (active_type == king) {
                printf("moved_type == king\n");
                b->king_moved[active_color] = true;
            }
            if (active_type == rook) {
                if (moved_x == 0)
                    b->left_rook_moved[active_color] = true;
                if (moved_x == 7)
                    b->right_rook_moved[active_color] = true;
            }

            // Move and make active cell empty
            Piece empty_piece = {.color = no_color, .type = no_type};
            touched->piece = b->active_cell->piece;
            b->active_cell->piece = empty_piece;

            // Move rook too if castled
            if (touched == b->left_castling_cell[active_color] ||
                touched == b->right_castling_cell[active_color]) {
                printf("touched castling cell\n");
                int dir = (touched == b->left_castling_cell[active_color]) ? 1 : -1;
                int x = (touched == b->left_castling_cell[active_color]) ? 0 : 7;
                b->cells[idx.y][idx.x + dir].piece.type = b->cells[idx.y][x].piece.type;
                b->cells[idx.y][idx.x + dir].piece.color = b->cells[idx.y][x].piece.color;
                b->cells[idx.y][x].piece.type = no_type;
                b->cells[idx.y][x].piece.color = no_color;
                b->left_rook_moved[active_color] = true;
            }

            b->move_pending = false;
            b->turn = (b->turn == black) ? white : black;

            // Handle promotions of pawns after moving if possible
            if (active_type == pawn) {
                int promoting_y = (active_color == black) ? 7 : 0;
                if (idx.y == promoting_y) {
                    printf("idx.y == promoting_y\n");
                    b->promotion_pending = true;
                    b->promoting_cell = touched;
                    return;
                }
            }

            recordDangerousCells(b);
            recordPins(b, b->turn);
            recordCheck(b);
            return;
        }
        else {
            printf("b->move_pendng && !touched->is_movable\n");
            b->move_pending = false;
        }
    }

    if (touched->piece.color != b->turn) {
        printf("touched->piece.color != b->turn\n");
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

    switch (touched->piece.type) {
    case pawn:
        fillPossibleMovesPawn(idx.x, idx.y, b);
        break;
    case rook:
    case bishop:
        fillPossibleMovesContinuous(idx.x, idx.y, touched->piece.type, b);
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

    // Special possible move for king (castling)
    // TODO: refactor
    enum PieceColor tcolor = touched->piece.color;
    b->left_castling_cell[tcolor] = NULL;
    b->right_castling_cell[tcolor] = NULL;
    if (touched->piece.type == king) {
        printf("touched->piece.type == king\n");
        bool empty_left =
            emptyCell(b->cells[idx.y][1]) &&
            emptyCell(b->cells[idx.y][2]) &&
            emptyCell(b->cells[idx.y][3]);
        bool empty_right =
        emptyCell(b->cells[idx.y][5]) &&
        emptyCell(b->cells[idx.y][6]);

        printf("empty in between: %d\n", empty_left);
        printf("b->king_moved[%s]: %d\n", tcolor == white ? "white" : "black", b->king_moved[tcolor]);
        printf("b->left_rook_moved[%s]: %d\n", tcolor == white ? "white" : "black", b->left_rook_moved[tcolor]);
        printf("b->right_rook_moved[%s]: %d\n", tcolor == white ? "white" : "black", b->right_rook_moved[tcolor]);

        if (!b->king_moved[tcolor]) {
            if (empty_left && !b->left_rook_moved[tcolor]) {
                b->cells[idx.y][2].in_range = true;
                // b->cells[idx.y][2].bg = LIGHTGRAY;
                b->left_castling_cell[tcolor] = &(b->cells[idx.y][2]);
                // b->move_pending = true;
            }
            if (empty_right && !b->right_rook_moved[tcolor]) {
                b->cells[idx.y][6].in_range = true;
                // b->cells[idx.y][6].bg = LIGHTGRAY;
                b->right_castling_cell[tcolor] = &(b->cells[idx.y][6]);
                // b->move_pending = true;
            }
        }
    }

    // Filter moves
    // printf("Filtration:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            Cell *cell = &(b->cells[i][j]);
            if (!cell->in_range)
                continue;

            // printf("\t%d %d:\n", i, j);
            if (cell->piece.color == touched->piece.color) {
                // printf("\t\tcell->piece.color == touched->piece.color\n");
                continue;
            }

            // Filter cells dangerous if king is moving
            if (touched->piece.type == king && cell->is_dangerous[touched->piece.color]) {
                // printf("\t\ttouched->piece.type == king && cell->is_dangerous[touched->piece.color]\n");
                continue;
            }

            // Filter cells that don't block check when some piece moves there
            if (b->king_checked && b->filter_nonblocking_cells && !touched->check_blocking_cells[i][j]) {
                // printf("\t\tb->king_checked && b->filter_nonblocking_cells && !touched->blocking_cells[i][j]\n");
                continue;
            }

            // Filter cells that might open a check to our king
            if (b->filter_check_opening && touched->opens_check && touched->check_opening_cells[i][j])
                continue;

            cell->is_movable = true;
            b->move_pending = true;

            cell->bg = emptyCell(*cell) ? BLUE : RED;
            if (cell == b->left_castling_cell[tcolor] || cell == b->right_castling_cell[tcolor])
                cell->bg = LIGHTGRAY;
        }
    }
}

void handlePromotion(int mouse_x, int mouse_y, Board *b, const PromotionWindow pwin)
{
    int diff = mouse_x - (pwin.pos.x + pwin.padding);
    if (diff < 0)
        return;

    int idx = diff / CELL_SIZE;
    if (idx >= 4)
        return;

    Piece chosen = {.type = pwin.promotables[idx], .color = b->promoting_cell->piece.color};
    b->promoting_cell->piece = chosen;
    b->promotion_pending = false;

    recordDangerousCells(b);
    recordPins(b, b->turn);
    recordCheck(b);
}

bool validCellIdx(int x, int y)
{
    return (0 <= x && x < 8) && (0 <= y && y < 8);
}

bool emptyCell(Cell c)
{
    return c.piece.type == no_type && c.piece.color == no_color;
}

void fillPossibleMovesPawn(int x, int y, Board *b)
{
    Cell touched = b->cells[y][x];

    // Direction of move: black goes down, white goes up
    int dir = (touched.piece.color == black) ? 1 : -1;
    int starting_pos = (touched.piece.color == black) ? 1 : 6;
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
    //         printf("\t\t\ttype: %d, color: %d\n", b->cells[i][j].piece.type, b->cells[i][j].piece.color);
    //     }
    // }


    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            // printf("\t%d %d\n", y, x);
            Cell c = b->cells[y][x];
            if (emptyCell(c))
                continue;

            enum PieceColor opposing = (c.piece.color == black) ? white : black;
            // printf("Before touch b:\n");
            // for (int i = 0; i < 8; i++) {
            //     printf("\t");
            //     for (int j = 0; j < 8; j++) {
            //         printf("%d ", b->cells[i][j].is_dangerous[opposing]);
            //     }
                // printf("\n");
            // }

            // Handle pawns differently
            if (c.piece.type == pawn) {
                // printf("c.piece.type == pawn\n");
                recordDangerousCellsByPawn(x, y, b);
                continue;
            }

            // Simulate touching the piece using a temporary board
            // Find cells that are in range, those cells will be dangerous
            // for opponent to enter
            Board tmp = *b;
            tmp.turn = c.piece.color;
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
    int dir = (touched.piece.color == black) ? 1 : -1;
    int j = y + dir;
    int xl = x - 1;
    int xr = x + 1;
    enum PieceColor dangerous_for = touched.piece.color == black ? white : black;
    if (validCellIdx(xl, j))
        b->cells[j][xl].is_dangerous[dangerous_for] = true;
    if (validCellIdx(xr, j))
        b->cells[j][xr].is_dangerous[dangerous_for] = true;
}


void recordPins(Board *b, enum PieceColor color)
{
    printf("recordPins()\n");

    // Reset
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->cells[y][x].opens_check = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    b->cells[y][x].check_opening_cells[i][j] = false;
        }
    }

    // Make a copy of board
    Board copy = *b;
    copy.turn = color;

    // Locate king's position
    V2 king_idx;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell c = b->cells[y][x];
            if (c.piece.type == king && c.piece.color == color) {
                king_idx.x = x;
                king_idx.y = y;
                break;
            }
        }
    }

    printf("king at : %d %d\n", king_idx.y, king_idx.x);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            Cell src = copy.cells[y][x];
            if (emptyCell(src) || src.piece.color != color || src.piece.type == king)
                continue;

            printf("\t%d %d\n", y, x);
            // Collect movable moves (dont filter check opening cells)
            // Otherwise everything may get filtered in first try
            Board tmp1 = copy;
            tmp1.filter_check_opening = false;
            tmp1.move_pending = false;
            handleMove(src.pos.x, src.pos.y, &tmp1);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    Cell dest = tmp1.cells[i][j];
                    if (!dest.is_movable)
                        continue;

                    printf("\t\t%d %d:\n", i, j);

                    Board tmp2 = tmp1;
                    Piece empty_piece = {.color = no_color, .type = no_type};
                    tmp2.cells[i][j].piece = tmp1.cells[y][x].piece;
                    tmp2.cells[y][x].piece = empty_piece;
                    recordDangerousCells(&tmp2);

                    // If king is in danger after moving, cell will open check to king
                    if (tmp2.cells[king_idx.y][king_idx.x].is_dangerous[color]) {
                        printf("\t\t\t%d %d is opening\n", i, j);
                        b->cells[y][x].opens_check = true;
                        b->cells[y][x].check_opening_cells[i][j] = true;
                        b->filter_check_opening = true;     // filter out check opening cells in further moves
                    }
                }
            }

            printf("Opening cells");
            for (int i = 0; i < 8; i++) {
                printf("\t\t");
                for (int j = 0; j < 8; j++) {
                    printf("%d ", b->cells[y][x].check_opening_cells[i][j]);
                }
                printf("\n");
            }
        }
    }
    return;
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
            if (c.piece.type == king && c.piece.color == king_color && c.is_dangerous[king_color]) {
                // printf("c.piece.type == king && c.piece.color == king_color && c.is_dangerous[king_color]\n");
                b->king_checked = true;
                b->checked_king = &(b->cells[i][j]);
                b->filter_nonblocking_cells = true;
                king_idx.y = i;
                king_idx.x = j;
            }
        }
    }

    if (!b->king_checked) {
        // printf("!b->king_checked\n");
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

            if (src.piece.color != king_color)
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

                    // printf("\tdest: %d %d:\n", i, j);

                    Board tmp2 = tmp1;
                    Piece empty_piece = {.color = no_color, .type = no_type};
                    tmp2.cells[i][j].piece = src.piece;
                    tmp2.cells[y][x].piece = empty_piece;

                    // for (int l = 0; l < 8; l++) {
                    //     for (int m = 0; m < 8; m++) {
                    //         printf("\t\ttype: %s, color: %s, idx: %d %d\n", getPieceString(tmp2.cells[l][m]), tmp2.cells[l][m].piece.color == white ? "white" : "black", l, m);
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
                    V2 king_at = (src.piece.type == king) ? (V2){.y = i, .x = j} : king_idx;
                    bool king_safe = !tmp2.cells[king_at.y][king_at.x].is_dangerous[king_color];
                    // printf("\tKing safe: %d\n", king_safe);
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