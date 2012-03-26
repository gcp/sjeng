/*
    Sjeng - a chess variants playing program
    Copyright (C) 2000 Gian-Carlo Pascutto

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    File: extvars.h                                        
    Purpose: global data definitions

*/

extern char divider[50];

extern int board[144], moved[144], ep_square, white_to_move, wking_loc,
  bking_loc, white_castled, black_castled, result, ply, pv_length[PV_BUFF],
  squares[144], num_pieces, i_depth, comp_color;

extern long int nodes, raw_nodes, qnodes, piece_count, killer_scores[PV_BUFF],
  killer_scores2[PV_BUFF], killer_scores3[PV_BUFF], moves_to_tc, min_per_game,
  inc, time_left, opp_time, time_cushion, time_for_move, cur_score;

extern unsigned long history_h[144][144];

extern bool xb_mode, captures, searching_pv, post, time_exit, time_failure;

extern move_s pv[PV_BUFF][PV_BUFF], dummy, killer1[PV_BUFF], killer2[PV_BUFF],
  killer3[PV_BUFF];

extern rtime_t start_time;

extern int holding[2][16];
extern int num_holding[2];

extern int hand_eval;

extern int drop_piece;

extern int pieces[62];
extern int is_promoted[62];

extern int num_makemoves;
extern int num_unmakemoves;
extern int num_playmoves;
extern int num_pieceups;
extern int num_piecedowns;
extern int max_moves;

/* piece types range form 0..16 */
extern unsigned long zobrist[17][144];
extern unsigned long hash;

extern unsigned long ECacheProbes;
extern unsigned long ECacheHits;

extern unsigned long TTProbes;
extern unsigned long TTHits;
extern unsigned long TTStores;

extern unsigned long hold_hash;

extern char book[4000][81];
extern int num_book_lines;
extern int book_ply;
extern int use_book;
extern char opening_history[STR_BUFF];

extern int Material;
extern int material[17];
extern int zh_material[17];
extern int std_material[17];

extern int NTries, NCuts, TExt;

extern char ponder_input[STR_BUFF];

extern bool is_pondering;

extern unsigned long DeltaTries, DeltaCuts;

extern unsigned long FH, FHF, PVS, FULL, PVSF;
extern unsigned long ext_check;
extern unsigned long razor_drop, razor_material;

extern unsigned long total_moves;
extern unsigned long total_movegens;

extern const int rank[144];
extern const int file[144];

extern int Variant;

extern bool is_analyzing;

extern char my_partner[STR_BUFF];
extern bool have_partner;
extern bool must_sit;
extern bool go_fast;
extern bool piecedead;
extern bool partnerdead;

extern char true_i_depth;

extern long fixed_time;

extern int hand_value[];

extern int numb_moves;

extern int phase;

FILE *lrn_standard;
FILE *lrn_zh;
extern int bestmovenum;

extern int ugly_ep_hack;



