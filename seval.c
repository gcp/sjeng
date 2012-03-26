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

    File: seval.c                                        
    Purpose: functions for evaluating positions (suicide chess)

*/

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"

static int scentral[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,-10,0,3,5,5,3,0,-10,0,0,
0,0,-10,0,15,15,15,15,0,-10,0,0,
0,0,-10,0,15,30,30,15,0,-10,0,0,
0,0,-10,0,15,30,30,15,0,-10,0,0,
0,0,-10,0,15,15,15,15,0,-10,0,0,
0,0,-10,0,3,5,5,3,0,-10,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

long int suicide_eval (void) {

  /* select the appropriate eval() routine: */
  return (suicide_mid_eval ());
}

long int suicide_mid_eval (void) {

  /* return a score for the current middlegame position: */

  int i, a, j;
  long int score = 0;
  int in_cache;

  in_cache = 0;
  
  checkECache(&score, &in_cache);
  
  if(in_cache)
    {
      if (white_to_move == 1) return score;
      return -score;
    } 

  score = Material;

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
	score += scentral[i];
	break;

      case (bpawn):
	score -= scentral[i];
	break;

      case (wrook):
	score += scentral[i];
	break;

      case (brook):
	score -= scentral[i];
	break;

      case (wbishop):
	score += scentral[i];
	break;

      case (bbishop):
	score -= scentral[i];
	break;

      case (wknight):
	score += scentral[i];
	break;

      case (bknight):
	score -= scentral[i];
	break;

      case (wqueen):
	score += scentral[i];
	break;

      case (bqueen):
	score -= scentral[i];
	break;

      case (wking):
	score += scentral[i] >> 1;
	break;

      case (bking):
	score -= scentral[i] >> 1;
	break;
    }
  }
  
  storeECache(score);
  
  /* adjust for color: */
  if (white_to_move == 1) {
    return score;
  }
  else {
    return -score;
  }

}
