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

    File: attacks.c                                             
    Purpose: calculate attack information                      
 
*/

#include "sjeng.h"
#include "extvars.h"

int calc_attackers (int square, int color) {

  /* this function calculates attack information for a square */

  static const int rook_o[4] = {12, -12, 1, -1};
  static const int bishop_o[4] = {11, -11, 13, -13};
  static const int knight_o[8] = {10, -10, 14, -14, 23, -23, 25, -25};
  int a_sq, i;

  int attackers = 0;

  if (board[square] == frame) return 0;

  /* white attacker: */
  if (color%2) {
    /* rook-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + rook_o[i];

      /* the king can attack from one square away: */
      if (board[a_sq] == wking) 
	{	  
	  attackers++;
	  break;
	}
      else
	{
	  /* otherwise, check for sliding pieces: */
	  while (board[a_sq] != frame) {
	    if (board[a_sq] == wrook || board[a_sq] == wqueen) 
	      {
		attackers++;
		break;
	      }
	    else if (board[a_sq] != npiece) break;
	    a_sq += rook_o [i];
	  }
	}
    }

    /* bishop-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + bishop_o[i];
      /* check for pawn attacks: */
      if (board[a_sq] == wpawn && i%2)
	{
	  attackers++;
	  break;
	}
      /* the king can attack from one square away: */
      else if (board[a_sq] == wking)
	{
	  attackers++;
	  break;
	}
      else
	{
	  while (board[a_sq] != frame) {
	    if (board[a_sq] == wbishop || board[a_sq] == wqueen) 
	      {
		attackers++;
		break;
	      }
	    else if (board[a_sq] != npiece) break;
	    a_sq += bishop_o [i];
	  }
	}
    }

    /* knight-style moves: */
    for (i = 0; i < 8; i++) {
      a_sq = square + knight_o[i];
      if (board[a_sq] == wknight)
	  attackers++;
	  
    }

    /* if we haven't hit a white attacker by now, there are none: */

  }

  /* black attacker: */
  else {
    /* rook-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + rook_o[i];
      /* the king can attack from one square away: */
      if (board[a_sq] == bking)
	{
	  attackers++;
	  break;
	}
      /* otherwise, check for sliding pieces: */
      else {
	while (board[a_sq] != frame) {
	  if (board[a_sq] == brook || board[a_sq] == bqueen) 
	    {
	      attackers++;
	      break;
	    };
	  if (board[a_sq] != npiece) break;
	  a_sq += rook_o [i];
	}
      }
    }

    /* bishop-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + bishop_o[i];
      /* check for pawn attacks: */
      if (board[a_sq] == bpawn && !(i%2))
	{
	  attackers++;
	  break;
	}
      /* the king can attack from one square away: */
      else if (board[a_sq] == bking)
	{
	  attackers++;
	  break;
	}
      else {
	while (board[a_sq] != frame) {
	  if (board[a_sq] == bbishop || board[a_sq] == bqueen) 
	    {
	      attackers++;
	      break;
	    }
	  else if (board[a_sq] != npiece) break;
	  a_sq += bishop_o [i];
	}
      }
    }

    /* knight-style moves: */
    for (i = 0; i < 8; i++) {
      a_sq = square + knight_o[i];
      if (board[a_sq] == bknight) 
	attackers++;
    }

    /* if we haven't hit a black attacker by now, there are none: */
  }

  return attackers;

}
