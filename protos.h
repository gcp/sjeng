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

    File: protos.h                                        
    Purpose: function prototypes

*/

#ifndef PROTOS_H
#define PROTOS_H

long int allocate_time (void);
bool check_legal (move_s moves[], int m);
void comp_to_coord (move_s move, char str[]);
void display_board (FILE *stream, int color);
long int end_eval (void);
long int eval (void);
long int seval(void);
void gen (move_s moves[], int *num_moves);
void ics_game_end (void);
bool in_check (void);
void init_game (void);
bool is_attacked (int square, int color);
bool is_move (char str[]);
void make (move_s moves[], int i);
void order_moves (move_s moves[], long int move_ordering[], int num_moves, int best);
long int mid_eval (void);
long int opn_eval (void);
void perft (int depth);
void perft_debug (void);
void post_thinking (long int score);
void post_fl_thinking (long int score, move_s *failmove);
void post_fh_thinking (long int score, move_s *failmove);
void post_fail_thinking(long int score, move_s *failmove);
void print_move (move_s moves[], int m, FILE *stream);
void push_king (move_s moves[], int *num_moves, int from, int target, 
		int castle_type);
void push_pawn (move_s moves[], int *num_moves, int from, int target,
	bool is_ep); 
void push_knight (move_s moves[], int *num_moves, int from, int target);

void try_drop (move_s moves[], int *num_moves, int ptype, int target);
		

void push_slide (move_s moves[], int *num_moves, int from, int target);
long int qsearch (int alpha, int beta, int depth);
void rdelay (int time_in_s);
long int rdifftime (rtime_t end, rtime_t start);
bool remove_one (int *marker, long int move_ordering[], int num_moves);
void reset_piece_square (void);
void rinput (char str[], int n, FILE *stream);
rtime_t rtime (void);
long int search (int alpha, int beta, int depth, bool is_null);
move_s search_root (int alpha, int beta, int depth);
void start_up (void);
move_s think (void);
void toggle_bool (bool *var);
void tree (int depth, int indent, FILE *output, char *disp_b);
void tree_debug (void);
void unmake (move_s moves[], int i);
bool verify_coord (char input[], move_s *move);


void ProcessHoldings(char line[]);
void addHolding(int what, int who);
void removeHolding(int what, int who);
void DropaddHolding(int what, int who);
void DropremoveHolding(int what, int who);

void printHolding(void);

int SwitchColor(int piece);
int SwitchPromoted(int piece);

int evalHolding(void);

void initialize_zobrist(void);
void initialize_hash(void);
void initialize_eval(void);

void checkECache(long int *score, int *in_cache);
void storeECache(long int score);

int init_book(void);
move_s choose_book_move(void);

void StoreTT(int score, int alpha, int beta, int best, int threat, int depth);
int ProbeTT(int *score, int alpha, int beta, int *best, int *threat, int *donull, int depth);
void LearnStoreTT(int score, unsigned nhash, unsigned hhash, int tomove, int best, int depth);

void LoadLearn(void);
void Learn(int score, int best, int depth);

void pinput (int n, FILE *stream);

int calc_attackers(int square, int color);

int interrupt(void);

void PutPiece(int color, char piece, char file, int rank);
void reset_board(void);

void reset_ecache(void);

void HandlePartner(char *input);
void HandlePtell(char *input);
void BegForPartner(void);
void CheckBadFlow(bool reset);

void run_epd_testsuite(void);

void ResetHandValue(void);
#endif

