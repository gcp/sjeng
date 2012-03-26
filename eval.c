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

    File: eval.c                                             
    Purpose: evaluate crazyhouse/bughouse positions

*/

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"

int Material;
int std_material[] = { 0, 100, -100, 310, -310, 4000, -4000, 300, -300, 900, -900, 325, -325, 0 }; 

int zh_material[] = { 0, 100, -100, 210, -210, 4000, -4000, 250, -250, 450, -450, 230, -230, 0 }; 

int suicide_material[] = { 0, 50, -50, 250, -250, 1000, -1000, 200, -200, 400, -400, 230, -230, 0 }; 

int material[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define max(x,y) (((x)>(y))?(x):(y))
#define min(x,y) (((x)>(y))?(y):(x))

const int file[144] =
{0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,1,2,3,4,5,6,7,8,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0};

const int rank[144] =
{0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,1,1,1,0,0,
 0,0,2,2,2,2,2,2,2,2,0,0,
 0,0,3,3,3,3,3,3,3,3,0,0,
 0,0,4,4,4,4,4,4,4,4,0,0,
 0,0,5,5,5,5,5,5,5,5,0,0,
 0,0,6,6,6,6,6,6,6,6,0,0,
 0,0,7,7,7,7,7,7,7,7,0,0,
 0,0,8,8,8,8,8,8,8,8,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0};

/* these tables will be used for positional bonuses: */

const int bishop[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-5,-5,-5,-5,-5,-5,-5,-5,0,0,
0,0,-5,10,5,10,10,5,10,-5,0,0,
0,0,-5,5,6,15,15,6,5,-5,0,0,
0,0,-5,3,15,10,10,15,3,-5,0,0,
0,0,-5,3,15,10,10,15,3,-5,0,0,
0,0,-5,5,6,15,15,6,5,-5,0,0,
0,0,-5,10,5,10,10,5,10,-5,0,0,
0,0,-5,-5,-5,-5,-5,-5,-5,-5,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int black_knight[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0, 20, 40, 45, 45, 45,  45,20,-10,0,0,
0,0,-10, 20, 45, 65, 45 , 65,20,-10,0,0,
0,0,-10, 10, 45, 30, 30,  35,35,-10,0,0,
0,0,-10, 0,  25, 30, 30,  25, 0,-10,0,0,
0,0,-10, 0,  25, 25, 25,  25, 0,-10,0,0,
0,0,-10, 0, 0,3,  3,  0,  0,-10,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int white_knight[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,-10,0,0,3,3,0,0,-10,0,0,
0,0,-10,0,25,25,25,25,0,-10,0,0,
0,0,-10,0,25,30,30,25,0,-10,0,0,
0,0,-10,10,45,30,30,35,35,-10,0,0,
0,0,-10,20,45,65,45,65,20,-10,0,0,
0,0, 20,40,45,45,45,45,45,20,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int white_pawn[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,50, 70,-20,-20, 70,50,0,0,0,
0,0,1,10,15,4,4,15,10,1,0,0,
0,0,2,4,6,8,30,30,4,2,0,0,
0,0,3,6,9,25,25,9,6,3,0,0,
0,0,4,18,25,25,25,25,18,4,0,0,
0,0,35,40,60,55,55,60,40,35,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int black_pawn[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,35,40,60,55,55,60,40,35,0,0,
0,0,4,18,25,25,25,25,18,4,0,0,
0,0,3,6,9,25,25,9,6,3,0,0,
0,0,2,4,6,8,30,30,4,2,0,0,
0,0,1,10,15,4,4,15,10,1,0,0,
0,0,0,50, 70,-20,-20, 70,50,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* to be used during opening and middlegame for white king positioning: */
const int white_king[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,   2,   5,   4,  15,  45  , 4,   5,   2,0,0,
0,0,-250,-200,-100, -50, -50,-100,-200,-250,0,0,
0,0,-350,-300,-300,-250,-250,-300,-300,-350,0,0,
0,0,-400,-400,-400,-350,-350,-400,-400,-400,0,0,
0,0,-450,-450,-450,-450,-450,-450,-450,-450,0,0,
0,0,-500,-500,-500,-500,-500,-500,-500,-500,0,0,
0,0,-500,-500,-500,-500,-500,-500,-500,-500,0,0,
0,0,-500,-500,-500,-500,-500,-500,-500,-500,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* to be used during opening and middlegame for black king positioning: */
const int black_king[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-500,-500,-500,-500,-500,-500,-500,-500,0,0,
0,0,-500,-500,-500,-500,-500,-500,-500,-500,0,0,
0,0,-500,-500,-500,-500,-500,-500,-500,-500,0,0,
0,0,-450,-450,-450,-450,-450,-450,-450,-450,0,0,
0,0,-400,-400,-400,-350,-350,-400,-400,-400,0,0,
0,0,-350,-300,-300,-250,-250,-300,-300,-350,0,0,
0,0,-250,-200,-100, -50, -50,-100,-200,-250,0,0,
0,0,   2,   5,   4,  15,  45  , 4,   5,   2,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int black_queen[144] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 2, 2, 2, 3,5,5,5,5, 0, 0,
0, 0, 0, 0, 0, 0, 0,5,5, 0, 0, 0,
0, 0,-15,-15,-15,-15,-15,-15,-15,-15,0, 0,
0,0,-30,-20,-20,-30,-30,-20,-20,-30,0,0,
0,0,-20,-20,-20,-20,-20,-20,-20,-20,0,0,
0,0,-15,-15,-15,-10,-10,-15,-15,-15,0,0,
0,0,0,0,0,7,10,5,0,0,0,0,
0,0,0,0,0, 5,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int white_queen[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,5,0,0,0,0,0,0,
0,0,0,0,0,7,10,5,0,0,0,0,
0,0,-15,-15,-15,-10,-10,-15,-15,-15,0,0,
0,0,-20,-20,-20,-20,-20,-20,-20,-20,0,0,
0,0,-30,-20,-20,-30,-30,-20,-20,-30,0,0,
0, 0,-15,-15,-15,-15,-15,-15,-15,-15,0, 0,
0, 0, 0, 0, 0, 0, 0,5,5, 0, 0, 0,
0, 0, 2, 2, 2, 3,5,5,5,5, 0, 0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

const int black_rook[144] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 10, 15, 20, 25,25,20,15,10, 0, 0,
0, 0, 0, 10, 15, 20, 20,15,10, 0, 0, 0,
0, 0,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,
0,0,-20,-20,-20,-30,-30,-20,-20,-20,0,0,
0,0,-20,-20,-20,-20,-20,-20,-20,-20,0,0,
0,0,-15,-15,-15,-10,-10,-15,-15,-15,0,0,
0,0,0,0,0,7,10,5,0,0,0,0,
0,0,0,0,0, 2,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,};

const int white_rook[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,2,0,0,0,0,0,0,
0,0,0,0,0,7,10,5,0,0,0,0,
0,0,-15,-15,-15,-10,-10,-15,-15,-15,0,0,
0,0,-20,-20,-20,-20,-20,-20,-20,-20,0,0,
0,0,-20,-20,-20,-30,-30,-20,-20,-20,0,0,
0, 0,-20,-20,-20,-20,-20,-20,-20,-20, 0, 0,
0, 0, 0,10,15,20,20,15,10, 0, 0, 0,
0, 0, 10, 15, 20, 25,25,20,15,10, 0, 0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* king safety tropisms */
/* tropism values of 0 and 8 are bogus, 
   and should never happen in the actual eval */

const int pre_p_tropism[9] = 
{ 9999, 120, 40, 20, 5, 2, 1, 0, 9999};

const int pre_r_tropism[9] = 
{ 9999, 100, 40, 15, 5, 2, 1, 0, 9999};

const int pre_n_tropism[9] =
{ 9999, 70, 90, 35, 10, 2, 1, 0, 9999};

const int pre_q_tropism[9] =
{ 9999, 120, 60, 20, 5, 2, 0, 0, 9999};

const int pre_b_tropism[9] =
{ 9999, 60, 25, 15, 5, 2, 2, 2, 9999};

unsigned char p_tropism[144][144];
unsigned char q_tropism[144][144];
unsigned char n_tropism[144][144];
unsigned char r_tropism[144][144];
unsigned char b_tropism[144][144];

void initialize_eval(void)
{
  int i, j;

  for(i = 0; i < 144; i++)
    {
      for(j = 0; j < 144; j++)
	{
	  p_tropism[i][j] = 
	    pre_p_tropism[max(abs(rank(i) - rank(j)), abs(file(i) - file(j)))];
	  b_tropism[i][j] = 
	    pre_b_tropism[max(abs(rank(i) - rank(j)), abs(file(i) - file(j)))];  
	  n_tropism[i][j] = 
	    pre_n_tropism[max(abs(rank(i) - rank(j)), abs(file(i) - file(j)))];  
	  r_tropism[i][j] = 
	    pre_r_tropism[max(abs(rank(i) - rank(j)), abs(file(i) - file(j)))];
	  q_tropism[i][j] = 
	    pre_q_tropism[max(abs(rank(i) - rank(j)), abs(file(i) - file(j)))];
	}
    }
}

long int eval (void) {

  /* return a score for the current middlegame position: */

  int i, a, j; 
  long int score = 0;
  int in_cache;
#ifdef FULLEVAL
  int safety = 0, attackers, badsquares = 0; 
#endif

  if (Variant == Normal)
    {
      return std_eval();
    }
  else if (Variant == Suicide)
    {
      return suicide_eval();
    }
  
  in_cache = 0;
  
  checkECache(&score, &in_cache);
  
  if(in_cache)
    {
      score += hand_eval;
      
      if (white_to_move == 1) return score;
      return -score;
    }

  /* loop through the board, adding material value, as well as positional
     bonuses for all pieces encountered: */
  for (a = 1, j = 1;(a <= piece_count); j++) {
    i = pieces[j];
    
    if (!i)
      continue;
    else
      a++;

    switch (board[i]) {
      case (wpawn):
	score += 100;
	score += white_pawn[i];
	score += p_tropism[i][bking_loc];
	break;

      case (bpawn):
	score -= 100;
	score -= black_pawn[i];
	score -= p_tropism[i][wking_loc];
	break;

      case (wrook):
	score += 250;
	score += white_rook[i];
	score += r_tropism[i][bking_loc];
	break;

      case (brook):
	score -= 250;
	score -= black_rook[i];
	score -= r_tropism[i][wking_loc];
	break;

      case (wbishop):
	score += 230;
	score += bishop[i];
	score += b_tropism[i][bking_loc];
	break;

      case (bbishop):
	score -= 230;
	score -= bishop[i];
	score -= b_tropism[i][wking_loc];
	break;

      case (wknight):
	score += 210;
	score += white_knight[i];
	score += n_tropism[i][bking_loc];
	break;

      case (bknight):
	score -= 210;
	score -= black_knight[i];
	score -= n_tropism[i][wking_loc];
	break;

      case (wqueen):
	score += 450;
	score += white_queen[i];
	score += q_tropism[i][bking_loc];
	break;

      case (bqueen):
	score -= 450;
	score -= black_queen[i];
	score -= q_tropism[i][wking_loc];
	break;

      case (wking):
	score += white_king[i];
	break;

      case (bking):
	score -= black_king[i];
	break;
    }
  }

  /* give penalties for blocking the e/d pawns: */
  if (!moved[41] && board[53] != npiece)
    score -= 15;
  if (!moved[42] && board[54] != npiece)
    score -= 15;
  if (!moved[101] && board[89] != npiece)
    score += 15;
  if (!moved[102] && board[90] != npiece)
    score += 15;

#undef FULLEVAL
#ifdef FULLEVAL
  /* white kingsafety */
  attackers = calc_attackers(wking_loc - 13, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc - 13, WHITE));
  attackers = calc_attackers(wking_loc - 12, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc - 12, WHITE));
  attackers = calc_attackers(wking_loc - 11, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc - 11, WHITE));
  attackers = calc_attackers(wking_loc - 1, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc - 1, WHITE));
  attackers = calc_attackers(wking_loc + 1, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc + 1, WHITE));
  attackers = calc_attackers(wking_loc + 11, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc + 11, WHITE));
  attackers = calc_attackers(wking_loc + 12, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc + 12, WHITE));
  attackers = calc_attackers(wking_loc + 13, BLACK);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(wking_loc + 13, WHITE));

  safety -= badsquares * 25;

  badsquares = 0;
  /* black ksafety */
  attackers = calc_attackers(bking_loc - 13, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc - 13, BLACK));
  attackers = calc_attackers(bking_loc - 12, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc - 12, BLACK));
  attackers = calc_attackers(bking_loc - 11, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc - 11, BLACK));
  attackers = calc_attackers(bking_loc - 1, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc - 1, BLACK));
  attackers = calc_attackers(bking_loc + 1, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc + 1, BLACK));
  attackers = calc_attackers(bking_loc + 13, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc + 13, BLACK));
  attackers = calc_attackers(bking_loc + 12, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc + 12, BLACK));
  attackers = calc_attackers(bking_loc + 11, WHITE);
  if (attackers)
    badsquares += max(0, attackers - calc_attackers(bking_loc + 11, BLACK));

  safety += badsquares * 25; 
 
  score += ((white_to_move==1)?safety:-safety);
#endif

  storeECache(score);

  score += hand_eval;

  /* adjust for color: */
  if (white_to_move == 1) {
    return score;
  }
  else {
    return -score;
  }

}





