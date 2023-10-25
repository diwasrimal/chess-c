#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
// #include <sys/time.h>

#include "tools.h"
#include "recorders.h"
#include "colorizers.h"
#include "fillers.h"

Board initBoard(void)
{
    char *starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board b = initBoardFromFEN(starting_fen);
    return b;
}

Board initBoardFromFEN(char *fen)
{
    if (fen == NULL)
        assert("fen == NULL\n");

    Board b;

    b.turn = white;
    b.move_count = 0;
    b.halfmove_clock = 0;
    b.fullmoves = 1;
    b.last_move = (Move){.src = NULL, .dst = NULL};
    b.computer_thinking = false;
    b.move_pending = false;
    b.promotion_pending = false;
    b.checkmate = false;
    b.king_checked = false;
    b.filter_nonblocking_cells = true;
    b.filter_check_opening = true;
    b.checked_king = NULL;
    b.active_cell = NULL;
    b.promoting_cell = NULL;
    b.queenside_castling_cell[black] = NULL;
    b.queenside_castling_cell[white] = NULL;
    b.kingside_castling_cell[black] = NULL;
    b.kingside_castling_cell[white] = NULL;
    b.en_passant_target = NULL;

    Piece empty_piece = {.type = no_type, .color = no_color};
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b.cells[y][x].piece = empty_piece;
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

    int i = 0;

    // Place pieces
    V2 idx = {.y = 0, .x = 0};
    enum PieceType types[256];
    types['K'] = king;
    types['Q'] = queen;
    types['N'] = knight;
    types['B'] = bishop;
    types['R'] = rook;
    types['P'] = pawn;
    while (fen[i] != ' ' && fen[i] != '\0') {
        char c = fen[i];
        if (c == '/') {
            idx.y++;
            idx.x = 0;
        }
        else if (isalpha(c)) {
            bool color = islower(c) ? black : white;
            int subscript = toupper(c);
            b.cells[idx.y][idx.x].piece = (Piece){.color = color, .type = types[subscript]};
            idx.x++;
        }
        else if (isdigit(c)) {
            int value = c - '0';
            idx.x += value;
        }
        i++;
    }

    // Set turn
    i++;
    b.turn = fen[i] == 'w' ? white : black;
    i += 2;

    // Set castling information
    b.queenside_castle_available[black] = false;
    b.queenside_castle_available[white] = false;
    b.kingside_castle_available[black] = false;
    b.kingside_castle_available[white] = false;
    while (fen[i] != ' ') {
        switch (fen[i]) {
        case 'K':
            b.kingside_castle_available[white] = true;
            break;
        case 'Q':
            b.queenside_castle_available[white] = true;
            break;
        case 'k':
            b.kingside_castle_available[black] = true;
            break;
        case 'q':
            b.queenside_castle_available[black] = true;
            break;
        default:
            break;
        }
        i++;
    }
    i++;

    // Record en passant information
    if (fen[i] == '-') {
        i += 2;
    } else {
        char file = fen[i++];
        char rank = fen[i++];
        int file_idx = file - 'a';
        int rank_idx = 8 - (rank - '0');
        b.en_passant_target = &b.cells[rank_idx][file_idx];
        i++;
    }

    b.halfmove_clock = 0;
    for (; fen[i] != ' '; i++) {
        b.halfmove_clock = b.halfmove_clock * 10 + (fen[i] - '0');
    }
    i++;

    b.fullmoves = 0;
    for (; fen[i] != '\0'; i++) {
        b.fullmoves = b.fullmoves * 10 + (fen[i] - '0');
    }

    resetCellBackgrounds(&b);
    recordDangerousCells(&b);
    return b;
}

void generateFEN(Board b)
{
    // Piece placing
    int i = 0;
    char piece_info[64];
    char notation[] = {'k', 'q', 'b', 'n', 'r', 'p'};
    int x = 0, y = 0;
    while (true) {
        if (emptyCell(b.cells[y][x]))  {
            int count = 0;
            while (emptyCell(b.cells[y][x])) {
                count++;
                x++;
                if (x == 8) {
                    piece_info[i++] = '0' + count;
                    piece_info[i++] = '/';
                    count = 0;
                    x = 0;
                    y++;
                }
            }
            if (count > 0)
                piece_info[i++] = '0' + count;
            if (y == 8)
                break;
        }

        Piece p = b.cells[y][x].piece;
        char nt = notation[p.type];
        if (p.color == white)
            nt = toupper(nt);
        piece_info[i++] = nt;
        x++;

        if (x == 8) {
            piece_info[i++] = '/';
            x = 0;
            y++;
        }

        if (y == 8)
            break;
    }

    piece_info[i] = '\0';
    if (piece_info[i - 1] == '/')
        piece_info[i - 1] = '\0';

    // Turn
    char turn = b.turn == white ? 'w' : 'b';


    // Castling info
    i = 0;
    char castling_info[5];
    if (b.kingside_castle_available[white])
        castling_info[i++] = 'K';
    if (b.queenside_castle_available[white])
        castling_info[i++] = 'Q';
    if (b.kingside_castle_available[black])
        castling_info[i++] = 'k';
    if (b.queenside_castle_available[black])
        castling_info[i++] = 'q';
    if (i == 0)
        castling_info[i++] = '-';
    castling_info[i] = '\0';

    // En passant target square
    i = 0;
    char en_passant_target[3];
    if (b.en_passant_target == NULL) {
        en_passant_target[i++] = '-';
    } else {
        char file = 'a' + b.en_passant_target->idx.x;
        char rank = '0' + (8 - b.en_passant_target->idx.y);
        en_passant_target[i++] = file;
        en_passant_target[i++] = rank;
    }
    en_passant_target[i++] = '\0';

    char fen[100];
    sprintf(fen, "%s %c %s %s %d %d", piece_info, turn, castling_info, en_passant_target, b.halfmove_clock, b.fullmoves);
    printf("FEN: %s\n", fen);
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

bool validCellIdx(int x, int y)
{
    return (0 <= x && x < 8) && (0 <= y && y < 8);
}

bool emptyCell(Cell c)
{
    return c.piece.type == no_type && c.piece.color == no_color;
}

void movePiece(Cell *from, Cell *to)
{
    if (from == NULL || to == NULL)
        assert(0 && "Cannot make move, from == NULL || to == NULL\n");

    Piece empty_piece = {.color = no_color, .type = no_type};
    to->piece = from->piece;
    from->piece = empty_piece;
}

void makeMove(const Move move, Board *b)
{
    enum PieceColor scolor = move.src->piece.color;
    enum PieceType stype = move.src->piece.type;
    V2 di = move.dst->idx;

    recordCastlingRightChanges(move, b);
    movePiece(move.src, move.dst);
    b->last_move = move;
    b->move_count++;
    b->halfmove_clock++;
    if (b->move_count % 2 == 0)
        b->fullmoves++;

    bool castled =
        stype == king && (move.dst == b->queenside_castling_cell[scolor] ||
                          move.dst == b->kingside_castling_cell[scolor]);
    if (castled) {
        int rook_dir = (move.dst == b->queenside_castling_cell[scolor]) ? 1 : -1;
        int rook_x = (move.dst == b->queenside_castling_cell[scolor]) ? 0 : 7;
        movePiece(&(b->cells[di.y][rook_x]), &(b->cells[di.y][di.x + rook_dir]));
        b->queenside_castle_available[scolor] = false;
        b->kingside_castle_available[scolor] = false;
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

void changeTurn(Board *b)
{
    b->turn = b->turn == black ? white : black;
}

// Assigns a numercial value to a board's state
float evaluateBoard(const Board b)
{
    // struct timeval t1, t2;
    // double elapsedTime;
    // gettimeofday(&t1, NULL);
    if (b.checkmate) {
        enum PieceColor losing = b.checked_king->piece.color;
        // gettimeofday(&t2, NULL);
        // elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        // elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        // printf("evaluation time: %f ms.\n", elapsedTime);
        return (losing == white) ? -400 : 400;
    }

    // printf("\nevaluateBoard()\n");
    enum PawnType {
        blocked,
        doubled,
        isolated,
    };
    int pawn_types[2][3] = {0};
    int count[2][6] = {0};            // piece count
    int legal_moves[2] = {0};

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell c = b.cells[y][x];
            if (emptyCell(c))
                continue;

            Piece p = c.piece;
            count[p.color][p.type]++;

            // Fill legal moves
            Board tmp = b;
            tmp.move_pending = false;
            tmp.active_cell = &(tmp.cells[y][x]);
            fillMovableCells(c, &tmp);
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++)
                    legal_moves[p.color] += (tmp.cells[i][j].is_movable);

            if (p.type == pawn) {
                int dir = p.color == black ? 1 : -1;

                // Blocked pawn
                if (!emptyCell(b.cells[y + dir][x])) {
                    // printf("pawn %d %d blocked\n", y, x);
                    pawn_types[p.color][blocked]++;
                }

                // Doubled pawn
                int dy = y + dir;
                while (validCellIdx(x, dy)) {
                    Cell c = b.cells[dy][x];
                    if (!emptyCell(b.cells[dy][x])) {
                        if (c.piece.type == pawn && c.piece.color == p.color) {
                            // printf("pawn %d %d doubled\n", y, x);
                            pawn_types[p.color][doubled]++;
                        }
                        break;
                    }
                    dy += dir;
                }

                // Isolated pawn (no friendly pawn in adjacent cols)
                bool isolated = true;
                if (validCellIdx(x - 1, y)) {
                    for (int j = 0; j < 8; j++) {
                        Piece p1 = b.cells[j][x - 1].piece;
                        if (p1.type == pawn && p1.color == p.color)
                            isolated = false;
                    }
                }
                if (validCellIdx(x + 1, y)) {
                    for (int j = 0; j < 8; j++) {
                        Piece p1 = b.cells[j][x + 1].piece;
                        if (p1.type == pawn && p1.color == p.color)
                            isolated = false;
                    }
                }
                if (isolated) {
                    // printf("pawn %d %d isolated\n", y, x);
                    pawn_types[p.color][isolated]++;
                }
            }
        }
    }

    // Weights of pieces
    float king_wt = 200;    // very high for handling checkmate but we handle checkmate at the beginning.
    float queen_wt = 9;
    float rook_wt = 5;
    float knight_wt = 3;
    float bishop_wt = 3;
    float pawn_wt = 1;

    // White is maximixing
    // https://www.chessprogramming.org/Evaluation
    float material_score =
        king_wt * (count[white][king] - count[black][king])
        + queen_wt * (count[white][queen] - count[black][queen])
        + rook_wt * (count[white][rook] - count[black][rook])
        + knight_wt * (count[white][knight] - count[black][knight])
        + bishop_wt * (count[white][bishop] - count[black][bishop])
        + pawn_wt * (count[white][pawn] - count[black][pawn])
        + king_wt * (count[white][king] - count[black][king])
        - 0.5 * (pawn_types[white][doubled] - pawn_types[black][doubled])
        - 0.5 * (pawn_types[white][blocked] - pawn_types[black][blocked])
        - 0.5 * (pawn_types[white][isolated] - pawn_types[black][isolated]);

    float mobility_score = 0.1 * (legal_moves[white] - legal_moves[black]);
    float total_score = material_score + mobility_score;
    // gettimeofday(&t2, NULL);
    // elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    // elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    // printf("evaluation time: %f ms.\n", elapsedTime);
    return total_score;
}

float minimax(Board b, int depth, bool is_maximizing, float alpha, float beta)
{
    if (depth == 0 || b.checkmate)
        return evaluateBoard(b);

    // int indent_level = 3 - depth;
    float best_score = is_maximizing ? INT_MIN : INT_MAX;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Cell src = b.cells[y][x];
            if (src.piece.color != b.turn || emptyCell(src))
                continue;

            Board unmoved = b;
            unmoved.move_pending = false;
            fillMovableCells(src, &unmoved);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    Cell dst = unmoved.cells[i][j];
                    if (!dst.is_movable)
                        continue;

                    // for (int k = 0; k < indent_level; k++) fprintf(log_file, "\t");
                    // fprintf(log_file, "%d%d -> %d%d:\n", y, x, i, j);

                    Board moved = unmoved;
                    moved.move_pending = true;
                    Move m = {.src = &(moved.cells[y][x]), .dst = &(moved.cells[i][j])};
                    makeMove(m, &moved);

                    float score = minimax(moved, depth - 1, !is_maximizing, alpha, beta);

                    // for (int k = 0; k < indent_level; k++) fprintf(log_file, "\t");
                    // fprintf(log_file, "score: %f\n", score);

                    if (is_maximizing) {
                        if (score > best_score) {
                            best_score = score;
                            alpha = score;
                        }
                        if (score > beta) {
                            return beta;
                        }
                    }
                    else {
                        if (score < best_score) {
                            best_score = score;
                            beta = score;
                        }
                        if (score < alpha) {
                            return alpha;
                        }
                    }
                }
            }
        }
    }
    return best_score;
}
