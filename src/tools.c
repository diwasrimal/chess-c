#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

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
    Board b;

    b.turn = white;
    b.move_count = 0;
    b.halfmove_clock = 0;
    b.fullmoves = 1;
    b.last_move = (Move){.src = NULL, .dst = NULL};
    b.move_pending = false;
    b.promotion_pending = false;
    b.checkmate = false;
    b.draw_by_fifty_move = false;
    b.draw_by_stalemate = false;
    b.king_checked = false;
    b.filter_nonblocking_cells = true;
    b.filter_check_opening = true;
    b.has_en_passant_target = false;
    b.checked_king = NULL;
    b.active_cell = NULL;
    b.promoting_cell = NULL;
    b.queenside_castling_cell[black] = NULL;
    b.queenside_castling_cell[white] = NULL;
    b.kingside_castling_cell[black] = NULL;
    b.kingside_castling_cell[white] = NULL;

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
        b.has_en_passant_target = false;
        i += 2;
    } else {
        char file = fen[i++];
        char rank = fen[i++];
        int file_idx = file - 'a';
        int rank_idx = 8 - (rank - '0');
        b.en_passant_target_idx = (V2){.y = rank_idx, .x = file_idx};
        b.has_en_passant_target = true;
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
    recordStateChangesAfterMove(&b);
    colorKingIfChecked(&b);
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
    if (b.has_en_passant_target) {
        char file = 'a' + b.en_passant_target_idx.x;
        char rank = '0' + (8 - b.en_passant_target_idx.y);
        en_passant_target[i++] = file;
        en_passant_target[i++] = rank;
    } else {
        en_passant_target[i++] = '-';
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
    V2 si = move.src->idx;
    V2 di = move.dst->idx;
    bool move_is_capturing = !emptyCell(*move.dst);

    recordCastlingRightChanges(move, b);
    movePiece(move.src, move.dst);
    b->last_move = move;

    b->move_count++;
    b->halfmove_clock++;
    if (b->move_count % 2 == 0)
        b->fullmoves++;

    // Captures or pawn movements reset halfmove clock
    if (stype == pawn || move_is_capturing)
        b->halfmove_clock = 0;

    // Move rook too if castled
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

    // Consume the double pushed pawn in case of en passant
    V2 epi = b->en_passant_target_idx;
    bool is_ep_capture = stype == pawn && (di.x == epi.x && di.y == epi.y);
    if (is_ep_capture) {
        int direction = (scolor == black) ? 1 : -1;
        int one_backwards = di.y - direction;
        b->cells[one_backwards][di.x].piece = (Piece){.type = no_type, .color = no_color};
    }

    // Play move sound
    if (move_is_capturing || is_ep_capture)
        PlaySound(sounds[capture_sound]);
    else
        PlaySound(sounds[move_sound]);

    b->move_pending = false;
    b->active_cell = NULL;
    changeTurn(b);

    b->has_en_passant_target = false;
    if (stype == pawn) {
        // Handle promotion of pawns
        int promoting_y = (scolor == black) ? 7 : 0;
        if (di.y == promoting_y) {
            b->promotion_pending = true;
            b->promoting_cell = move.dst;
            return;
        }

        // Record en passant target in case of double pawn push
        int starting_y = (scolor == black) ? 1 : 6;
        int direction = (scolor == black) ? 1 : -1;
        int two_forward = starting_y + 2 * direction;
        bool is_double_pawn_push = (si.y == starting_y && di.y == two_forward);
        if (is_double_pawn_push) {
            b->en_passant_target_idx = (V2){.y = starting_y + direction, .x = si.x};
            b->has_en_passant_target = true;
        }
    }

    recordStateChangesAfterMove(b);
}

void changeTurn(Board *b)
{
    b->turn = b->turn == black ? white : black;
}
