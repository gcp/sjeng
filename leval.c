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

    File: leval.c                                        
    Purpose: functions for evaluating positions in losers chess

*/

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"

static int lcentral[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-20,-15,-15,-15,-15,-15,-15,-20,0,0,
0,0,-15,0,3,5,5,3,0,-15,0,0,
0,0,-15,0,15,15,15,15,0,-15,0,0,
0,0,-15,0,15,30,30,15,0,-15,0,0,
0,0,-15,0,15,30,30,15,0,-15,0,0,
0,0,-15,0,15,15,15,15,0,-15,0,0,
0,0,-15,0,3,5,5,3,0,-15,0,0,
0,0,-20,-15,-15,-15,-15,-15,-15,-20,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

static int l_bishop_mobility(int square)
{
  register int l;
  register int m = 0;

  for (l = square-13; board[l] == npiece; l-=13)
    m++;
  for (l = square-11; board[l] == npiece; l-=11)
    m++;
  for (l = square+11; board[l] == npiece; l+=11)
    m++;
  for (l = square+13; board[l] == npiece; l+=13)
    m++;

  return m;
}

static int l_rook_mobility(int square)
{
  register int l;
  register int m = 0;

  for (l = square-12; board[l] == npiece; l-=12)
    m++;
  for (l = square-1; board[l] == npiece; l-=1)
    m++;
  for (l = square+1; board[l] == npiece; l+=1)
    m++;
  for (l = square+12; board[l] == npiece; l+=12)
    m++;

  return m;
}

long int losers_eval (void) {

  /* return a score for the current middlegame position: */

  int i, a, j;
  long int score = 0;
  int in_cache;
  int wp = 0, bp = 0;
  
  in_cache = 0;
  
  checkECache(&score, &in_cache);
  
  if(in_cache)
    {
      if (white_to_move == 1) return score;
      return -score;
    } 


  if (abs(Material) <= 900)
  {
    score = Material;
  }
  else
  {
    /* one side has a huge advantage, which could
     * become problematic */
    /* only apply this to self, we assume somebody
     * else can handle this just fine */
    
    if (Material > 0 && comp_color == WHITE)
    {
       score = 1800 - Material;
    }
    else if (Material < 0 && comp_color == BLACK)
    {
       score = -(1800 + Material);
    }
    else
    {
    	score = Material;
    }
  }

  /* loop through the board, adding material value, as well as positional
     bonuses for all pieces encountered: */
  for (j = 1, a = 1; (a <= piece_count); j++) {
    i = pieces[j];
    
    if (!i)
      continue;
    else
      a++;

    switch (board[i]) {
      case (wpawn):
	wp++;
	score += lcentral[i];
	score += (rank(i) - 2) * 10;
	break;

      case (bpawn):
	bp++;
	score -= lcentral[i];
	score -= (7 - rank(i)) * 10;
	break;

      case (wrook):
	wp++;
	score += l_rook_mobility(i) << 1;
	score += lcentral[i];
	break;

      case (brook):
	bp++;
	score -= l_rook_mobility(i) << 1;
	score -= lcentral[i];
	break;

      case (wbishop):
	wp++;
	score += l_bishop_mobility(i) << 1;
	score += lcentral[i];
	break;

      case (bbishop):
	bp++;
	score -= l_bishop_mobility(i) << 1;
	score -= lcentral[i];
	break;

      case (wknight):
	wp++;
	score += lcentral[i] << 1;
	break;

      case (bknight):
	bp++;
	score -= lcentral[i] << 1;
	break;

      case (wqueen):
	wp++;
	score += l_bishop_mobility(i) << 1;
	score += l_rook_mobility(i) << 1;
	score += lcentral[i];
	break;

      case (bqueen):
	bp++;
	score -= l_bishop_mobility(i) << 1;
	score -= l_rook_mobility(i) << 1;
	score -= lcentral[i];
	break;

      case (wking):
	/* being in center is BAD */
	score -= lcentral[i] << 1;
	break;

      case (bking):
	/* being in center is BAD */
	score += lcentral[i] << 1;
	break;
    }
  }

  if (!wp) score = INF;
  else if (!bp) score = -INF;
  
  storeECache(score);
  
  /* adjust for color: */
  if (white_to_move == 1) {
    return score;
  }
  else {
    return -score;
  }

}









