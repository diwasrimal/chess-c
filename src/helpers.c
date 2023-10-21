#include "helpers.h"

Board initBoard(void)
{
    Board b;

    resetCellBackgrounds(&b);
    b.turn = white;
    b.move_count = 0;
    b.last_move = (Move){.src = NULL, .dst = NULL};
    b.move_pending = false;
    b.promotion_pending = false;
    b.checkmate = false;
    b.king_checked = false;
    b.filter_nonblocking_cells = true;
    b.filter_check_opening = true;
    b.left_castle_possible[black] = true;
    b.left_castle_possible[white] = true;
    b.right_castle_possible[black] = true;
    b.right_castle_possible[white] = true;
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
            b.cells[y][x].idx.y = y;
            b.cells[y][x].idx.x = x;
            b.cells[y][x].in_range = false;
            b.cells[y][x].is_movable = false;
            b.cells[y][x].is_dangerous[black] = false;
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
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            b->cells[y][x].bg = checkers[(y + x) % 2];
}

void colorKingIfChecked(Board *b)
{
    if (b->king_checked)
        recolorCell(b->checked_king, COLOR_CELL_CAPTURABLE);
}

void colorLastMove(Board *b)
{
    if (b->move_count == 0)
        return;
    recolorCell(b->last_move.src, COLOR_MOVE_SRC);
    recolorCell(b->last_move.dst, COLOR_MOVE_DST);
}

void decolorLastMove(Board *b)
{
    if (b->move_count == 0)
        return;
    Cell *src = b->last_move.src;
    Cell *dst = b->last_move.dst;
    src->bg = checkers[(src->idx.y + src->idx.x) % 2];
    dst->bg = checkers[(dst->idx.y + dst->idx.x) % 2];
}

// Applies a color on top of an exisiting background color
void recolorCell(Cell *cell, Color color)
{
    Color original = checkers[(cell->idx.y + cell->idx.x) % 2];
    bool cell_is_dark =
        original.r == COLOR_CHECKER_DARK.r &&
        original.g == COLOR_CHECKER_DARK.g &&
        original.b == COLOR_CHECKER_DARK.b;

    color.a = cell_is_dark ? 255 : 200;
    cell->bg = color;
}

void handleTouch(int mouse_x, int mouse_y, Board *b)
{
    // Always color these
    resetCellBackgrounds(b);
    colorKingIfChecked(b);
    colorLastMove(b);

    V2 ti = cellIdxByPos(mouse_x, mouse_y);     // touched idx
    Cell *touched = &(b->cells[ti.y][ti.x]);
    enum PieceColor tcolor = touched->piece.color;

    if (b->move_pending) {
        if (touched->is_movable) {

            Move move = {.src = b->active_cell, .dst = touched};
            decolorLastMove(b);
            makeMove(move, b);
            colorLastMove(b);


            return;
        }
        else {
            b->move_pending = false;
        }
    }

    if (tcolor != b->turn)
        return;

    if (emptyCell(*touched))
        return;

    recolorCell(touched, COLOR_CELL_ACTIVE);
    b->active_cell = touched;
    b->move_pending = false;

    fillMovableCells(*touched, b);
    colorMovableCells(*touched, b);
}

void fillMovableCells(const Cell touched, Board *b)
{
    // Reset cells in range and cells that are movable
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            b->cells[i][j].in_range = false;
            b->cells[i][j].is_movable = false;
        }
    }

    fillCellsInRange(touched, b);
    filterCellsInRange(touched, b);
}

void fillCellsInRange(const Cell touched, Board *b)
{

    enum PieceType ttype = touched.piece.type;
    enum PieceColor tcolor = touched.piece.color;

    switch (ttype) {
    case pawn:
        fillCellsInRangePawn(touched, b);
        break;
    case rook:
    case bishop:
        fillCellsInRangeContinuous(touched, ttype, b);
        break;
    case queen:
        fillCellsInRangeContinuous(touched, rook, b);
        fillCellsInRangeContinuous(touched, bishop, b);
        break;
    case knight:
        fillCellsInRangeKnight(touched, b);
        break;
    case king:
        fillCellsInRangeKing(touched, b);
        break;
    default:
        fprintf(stderr, "Not implemented!\n");
    }

    if (ttype == king) {
        b->left_castling_cell[tcolor] = NULL;
        b->right_castling_cell[tcolor] = NULL;
        fillCastlingCells(touched, b);
    }
}

void filterCellsInRange(const Cell touched, Board *b)
{
    enum PieceType ttype = touched.piece.type;
    enum PieceColor tcolor = touched.piece.color;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            Cell *cell = &(b->cells[i][j]);
            if (!cell->in_range)
                continue;

            // Filter pieces of same color
            if (cell->piece.color == tcolor) {
                continue;
            }

            // Filter cells dangerous if king is moving
            if (ttype == king && cell->is_dangerous[tcolor]) {
                continue;
            }

            // Filter cells that don't block check when some piece moves there
            if (b->king_checked && b->filter_nonblocking_cells && !touched.check_blocking_cells[i][j]) {
                continue;
            }

            // Filter cells that might open a check to our king
            if (b->filter_check_opening && touched.opens_check && touched.check_opening_cells[i][j])
                continue;

            cell->is_movable = true;
            b->move_pending = true;
        }
    }
}

void handlePromotion(int mouse_x, int mouse_y, Board *b, const PromotionWindow pwin)
{
    if (!b->promotion_pending)
        assert(0 && "!b->promotion_pending\n");

    int fx = pwin.first_cell_pos.x;
    int fy = pwin.first_cell_pos.y;
    int lx = fx + pwin.cell_margin * 3 + CELL_SIZE * 4;
    int ly = fy + CELL_SIZE;

    // Touched outside promotion choosing window
    if (mouse_x < fx || mouse_x > lx || mouse_y < fy || mouse_y > ly)
        return;

    int idx = (mouse_x - fx) / CELL_SIZE;
    Piece chosen = {.type = pwin.promotables[idx], .color = b->promoting_cell->piece.color};
    b->promoting_cell->piece = chosen;
    b->promotion_pending = false;
    b->promoting_cell = NULL;

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

void fillCellsInRangePawn(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceColor tcolor = touched.piece.color;
    enum PieceType ttype = touched.piece.type;

    if (ttype != pawn)
        assert(0 && "ttype != pawn\n");

    // Direction of move: black goes down, white goes up
    int dir = (tcolor == black) ? 1 : -1;
    int starting_pos = (tcolor == black) ? 1 : 6;
    bool in_starting_position = ti.y == starting_pos;

    // Straight move (should be empty)
    int move_limit = in_starting_position ? 2 : 1;
    for (int dy = 1; dy <= move_limit; dy++) {
        int j = ti.y + dy * dir;
        if (!validCellIdx(ti.x, j) || !emptyCell(b->cells[j][ti.x]))
            break;
        b->cells[j][ti.x].in_range = true;
    }

    // Diagonal moves (only captures)
    int j = ti.y + dir;
    int xl = ti.x - 1;
    int xr = ti.x + 1;

    if (validCellIdx(xl, j) && !emptyCell(b->cells[j][xl]))
        b->cells[j][xl].in_range = true;

    if (validCellIdx(xr, j) && !emptyCell(b->cells[j][xr]))
        b->cells[j][xr].in_range = true;
}


void fillCellsInRangeContinuous(const Cell touched, enum PieceType ttype, Board *b)
{
    V2 ti = touched.idx;

    bool moves_continuous = (ttype == rook || ttype == bishop || ttype == queen);
    if (!moves_continuous)
        assert(0 && "!moves_continuous\n");

    V2 vectors[2][2];
    switch (ttype) {
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
        fprintf(stderr, "Not implemented continuous move for type: %d\n", ttype);
    }

    for (int l = 0; l < 2; l++) {
        for (int m = 0; m < 2; m++) {
            V2 vec = vectors[l][m];
            int i = ti.x + vec.x;
            int j = ti.y + vec.y;

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

void fillCellsInRangeKnight(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceType ttype = touched.piece.type;

    if (ttype != knight)
        assert(0 && "ttype != knight\n");

    int dy[8] = {2, 2, -2, -2, 1, 1, -1, -1};
    int dx[8] = {-1, 1, -1, 1, -2, 2, -2, 2};
    for (int k = 0; k < 8; k++) {
        int i = ti.x + dx[k];
        int j = ti.y + dy[k];
        if (!validCellIdx(i, j)) continue;
        b->cells[j][i].in_range = true;
    }
}

void fillCellsInRangeKing(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceType ttype = touched.piece.type;

    if (ttype != king)
        assert(0 && "ttype != king\n");

    for (int j = ti.y - 1; j <= ti.y + 1; j++) {
        for (int i = ti.x - 1; i <= ti.x + 1; i++) {
            if ((i == ti.x && j == ti.y) || !validCellIdx(i, j))
               continue;
            b->cells[j][i].in_range = true;
        }
    }
}

void fillCastlingCells(const Cell touched, Board *b)
{
    V2 ti = touched.idx;
    enum PieceColor tcolor = touched.piece.color;
    enum PieceType ttype = touched.piece.type;

    if (ttype != king)
        assert(0 && "ttype != king, cannot fill castling cells\n");

    bool empty_left = emptyCell(b->cells[ti.y][1]) &&
                      emptyCell(b->cells[ti.y][2]) &&
                      emptyCell(b->cells[ti.y][3]);
    bool empty_right =
        emptyCell(b->cells[ti.y][5]) && emptyCell(b->cells[ti.y][6]);

    if (b->left_castle_possible[tcolor] && empty_left) {
        b->cells[ti.y][2].in_range = true;
        b->left_castling_cell[tcolor] = &(b->cells[ti.y][2]);
    }
    if (b->right_castle_possible[tcolor] && empty_right) {
        b->cells[ti.y][6].in_range = true;
        b->right_castling_cell[tcolor] = &(b->cells[ti.y][6]);
    }
}

void changeTurn(Board *b)
{
    b->turn = b->turn == black ? white : black;
}

void makeMove(const Move move, Board *b)
{
    enum PieceColor scolor = move.src->piece.color;
    enum PieceType stype = move.src->piece.type;
    V2 di = move.dst->idx;

    recordCastlingPossibility(move, b);
    movePiece(move.src, move.dst);
    b->last_move = move;
    b->move_count++;

    bool castled =
        stype == king && (move.dst == b->left_castling_cell[scolor] ||
                          move.dst == b->right_castling_cell[scolor]);
    if (castled) {
        int rook_dir = (move.dst == b->left_castling_cell[scolor]) ? 1 : -1;
        int rook_x = (move.dst == b->left_castling_cell[scolor]) ? 0 : 7;
        movePiece(&(b->cells[di.y][rook_x]), &(b->cells[di.y][di.x + rook_dir]));
        b->left_castle_possible[scolor] = false;
        b->right_castle_possible[scolor] = false;
    }

    b->move_pending = false;
    b->active_cell = NULL;
    changeTurn(b);

    // Handle promotions of pawns after moving if possible
    if (stype == pawn) {
        int promoting_y = (scolor == black) ? 7 : 0;
        if (move.dst->idx.y == promoting_y) {
            b->promotion_pending = true;
            b->promoting_cell = move.dst;
            return;
        }
    }

    recordDangerousCells(b);
    recordPins(b, b->turn);
    recordCheck(b);
}

void movePiece(Cell *from, Cell *to)
{
    if (from == NULL || to == NULL)
        assert(0 && "Cannot make move, from == NULL || to == NULL\n");

    Piece empty_piece = {.color = no_color, .type = no_type};
    to->piece = from->piece;
    from->piece = empty_piece;
}

// Records change in castling possibilities when given move is made
void recordCastlingPossibility(Move m, Board *b)
{
    V2 si = m.src->idx;
    V2 di = m.dst->idx;
    enum PieceType stype = m.src->piece.type;
    enum PieceType dtype = m.dst->piece.type;
    enum PieceColor scolor = m.src->piece.color;
    enum PieceColor dcolor = m.dst->piece.color;

    // King moves
    if (stype == king) {
        b->left_castle_possible[scolor] = false;
        b->right_castle_possible[scolor] = false;
    }

    // Rook moves
    if (stype == rook && si.x == 0)
        b->left_castle_possible[scolor] = false;
    if (stype == rook && si.x == 7)
        b->right_castle_possible[scolor] = false;

    // Rook capture
    if (dtype == rook && di.x == 0)
        b->left_castle_possible[dcolor] = false;
    if (dtype == rook && di.x == 7)
        b->right_castle_possible[dcolor] = false;
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
            if (emptyCell(c))
                continue;

            enum PieceColor opposing = (c.piece.color == black) ? white : black;

            // Handle pawns differently
            if (c.piece.type == pawn) {
                recordDangerousCellsByPawn(x, y, b);
                continue;
            }

            // Simulate touching the piece using a temporary board
            // Find cells that are in range, those cells will be dangerous
            // for opponent to enter
            Board tmp = *b;
            tmp.turn = c.piece.color;
            tmp.move_pending = false;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    tmp.cells[i][j].in_range = false;

            fillCellsInRange(c, &tmp);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (tmp.cells[i][j].in_range)
                        b->cells[i][j].is_dangerous[opposing] = true;
                }
            }
        }
    }
}

void colorMovableCells(const Cell touched, Board *b)
{
    enum PieceColor tcolor = touched.piece.color;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Cell *cell = &(b->cells[i][j]);
            if (!cell->is_movable)
                continue;

            Color newbg = !emptyCell(*cell) ? COLOR_CELL_CAPTURABLE : COLOR_CELL_MOVABLE;
            if (cell == b->left_castling_cell[tcolor] || cell == b->right_castling_cell[tcolor])
                newbg = COLOR_CELL_CASTLING;

            recolorCell(cell, newbg);
        }
    }
}

void recordDangerousCellsByPawn(int x, int y, Board *b)
{
    // Pawn captures only diagonals, thus threatens only diagonals
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

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            Cell src = copy.cells[y][x];
            if (emptyCell(src) || src.piece.color != color || src.piece.type == king)
                continue;

            // Collect movable moves (dont filter check opening cells)
            // Otherwise everything may get filtered in first try
            Board tmp1 = copy;
            tmp1.filter_check_opening = false;
            tmp1.move_pending = false;
            fillMovableCells(src, &tmp1);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    Cell dst = tmp1.cells[i][j];
                    if (!dst.is_movable)
                        continue;

                    Board tmp2 = tmp1;
                    movePiece(&tmp2.cells[y][x], &tmp2.cells[i][j]);
                    recordDangerousCells(&tmp2);

                    // If king is in danger after moving, cell will open check to king
                    if (tmp2.cells[king_idx.y][king_idx.x].is_dangerous[color]) {
                        b->cells[y][x].opens_check = true;
                        b->cells[y][x].check_opening_cells[i][j] = true;
                        b->filter_check_opening = true;     // filter out check opening cells in further moves
                    }
                }
            }
        }
    }
    return;
}

// Finds whether king is checked, and finds cells that can block the check
void recordCheck(Board *b)
{
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
                b->king_checked = true;
                b->checked_king = &(b->cells[i][j]);
                b->filter_nonblocking_cells = true;
                king_idx.y = i;
                king_idx.x = j;
            }
        }
    }

    if (!b->king_checked)
        return;

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
            Board tmp1 = *b;
            tmp1.move_pending = false;
            tmp1.filter_nonblocking_cells = false;
            handleTouch(src.pos.x, src.pos.y, &tmp1);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {

                    Cell dest = tmp1.cells[i][j];
                    if (!dest.is_movable)
                        continue;

                    Board tmp2 = tmp1;
                    Piece empty_piece = {.color = no_color, .type = no_type};
                    tmp2.cells[i][j].piece = src.piece;
                    tmp2.cells[y][x].piece = empty_piece;
                    recordDangerousCells(&tmp2);

                    // If king is safe now, src blocks the check
                    // If src was king, king is at (i, j) now
                    V2 king_at = (src.piece.type == king) ? (V2){.y = i, .x = j} : king_idx;
                    bool king_safe = !tmp2.cells[king_at.y][king_at.x].is_dangerous[king_color];
                    if (king_safe) {
                        can_be_blocked = true;
                        b->cells[y][x].blocks_check = true;
                        b->cells[y][x].check_blocking_cells[i][j] = true;
                    }
                }
            }
        }
    }

    if (!can_be_blocked)
        b->checkmate = true;
}