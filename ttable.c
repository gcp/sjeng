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

    File: ttable.c                                       
    Purpose: handling of transposition tables and hashes

*/

#include "sjeng.h"
#include "protos.h"
#include "extvars.h"
#include "limits.h"

unsigned long zobrist[17][144];

unsigned long hash;

unsigned long TTProbes;
unsigned long TTHits;
unsigned long TTStores;

#define TTSIZE 1000000

typedef struct 
{
  signed char Depth;  
  /* unsigned char may be a bit small for bughouse/crazyhouse */
  unsigned char Bestmove;
  unsigned OnMove:1, Threat:1, Type:2;
  unsigned long Hash;
  unsigned long Hold_hash;
  signed long Bound;
}
TType;

TType TTable[TTSIZE];

void clear_tt(void)
{
  memset(TTable, 0, sizeof(TTable));
};

void initialize_zobrist(void)
{
  int p, q;

  srand(1234);

  for(p = 0; p < 17; p++)
    for(q = 0; q < 144; q++)
      {
	/* rand might return a 16 or a 32 bit integer */
	zobrist[p][q] = (rand() << 16) + rand();
      }

  /* our magic number */

  hash = 0xDEADBEEF;
}

void initialize_hash(void)
{
  int p;
  
  hash = 0xDEADBEEF;
  
  for(p = 0; p < 144; p++)
    {
      hash = hash ^ zobrist[board[p]][p];
    }

  hold_hash = 0xC0FFEE00;
  /* we need to set up hold_hash here, reply on ProcessHolding for now */

}

void StoreTT(int score, int alpha, int beta, int best, int threat, int depth)
{
  unsigned long index;
  
  TTStores++;

  index = hash % TTSIZE;

  if (score <= alpha)     
    TTable[index].Type = UPPER;
  else if(score >= beta) 
    TTable[index].Type = LOWER;
  else                  
    TTable[index].Type = EXACT;

  /* normalize mate scores */
  if (score > (+INF-500))
    score += ply;
  else if (score < (-INF+500))
    score -= ply;

  TTable[index].Hash = hash;
  TTable[index].Hold_hash = hold_hash;
  TTable[index].Depth = depth;
  TTable[index].Bestmove = best;
  TTable[index].Bound = score;
  TTable[index].OnMove = ToMove;
  TTable[index].Threat = threat;

  return;
}

void LearnStoreTT(int score, unsigned nhash, unsigned hhash, int tomove, int best, int depth)
{
  unsigned long index;

  index = nhash % TTSIZE;

  if (TTable[index].Depth <= depth)
    TTable[index].Depth = depth;
  else
    return;
     
  TTable[index].Type = EXACT;
  TTable[index].Hash = nhash;
  TTable[index].Hold_hash = hhash;
  TTable[index].Bestmove = best;
  TTable[index].Bound = score;
  TTable[index].OnMove = tomove;
  TTable[index].Threat = 0;

}

int ProbeTT(int *score, int alpha, int beta, int *best, int *threat, int *donull, int depth)
{

  unsigned long index;

  *donull = TRUE;

  TTProbes++;

  index = hash % TTSIZE;
  
  if ((TTable[index].Hash == hash) 
      && (TTable[index].Hold_hash == hold_hash) 
      && (TTable[index].OnMove == ToMove))
    {
      TTHits++;
      
      /*if ((TTable[index].Type == UPPER) 
      	   && ((depth-2-1) <= TTable[index].Depth) 
      	   && (TTable[index].Bound < beta)) 
      	  *donull = FALSE;*/

      if (TTable[index].Depth >= depth)
	{
	  *score = TTable[index].Bound;
	  
	  if (*score > (+INF-500))
	   *score -= ply;
	  else if (*score < (-INF+500))
	    *score += ply;

	  *best = TTable[index].Bestmove;
	  *threat = TTable[index].Threat;

	  return TTable[index].Type;
	}
      else
	{
	  *best = TTable[index].Bestmove;
	  *threat = TTable[index].Threat;

	  return DUMMY;
	}
    }
  else
    return HMISS;

}
