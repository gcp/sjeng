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

#define TTSIZE 500000

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

TType DP_TTable[TTSIZE];
TType AS_TTable[TTSIZE];

void clear_tt(void)
{
  memset(DP_TTable, 0, sizeof(DP_TTable));
  memset(AS_TTable, 0, sizeof(AS_TTable));
};

void clear_dp_tt(void)
{
  memset(DP_TTable, 0, sizeof(DP_TTable));
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
  /* we need to set up hold_hash here, rely on ProcessHolding for now */

}

void StoreTT(int score, int alpha, int beta, int best, int threat, int depth)
{
  unsigned long index;
  
  TTStores++;

  index = hash % TTSIZE;

  if (DP_TTable[index].Depth <= depth)
    {
      if (score <= alpha)     
	DP_TTable[index].Type = UPPER;
      else if(score >= beta) 
	DP_TTable[index].Type = LOWER;
      else                  
	DP_TTable[index].Type = EXACT;
      
      /* normalize mate scores */
      if (score > (+INF-500))
	score += ply;
      else if (score < (-INF+500))
	score -= ply;
      
      DP_TTable[index].Hash = hash;
      DP_TTable[index].Hold_hash = hold_hash;
      DP_TTable[index].Depth = depth;
      DP_TTable[index].Bestmove = best;
      DP_TTable[index].Bound = score;
      DP_TTable[index].OnMove = ToMove;
      DP_TTable[index].Threat = threat;
    }
  else 
    {
     if (score <= alpha)     
	AS_TTable[index].Type = UPPER;
      else if(score >= beta) 
	AS_TTable[index].Type = LOWER;
      else                  
	AS_TTable[index].Type = EXACT;
      
      /* normalize mate scores */
      if (score > (+INF-500))
	score += ply;
      else if (score < (-INF+500))
	score -= ply;
      
      AS_TTable[index].Hash = hash;
      AS_TTable[index].Hold_hash = hold_hash;
      AS_TTable[index].Depth = depth;
      AS_TTable[index].Bestmove = best;
      AS_TTable[index].Bound = score;
      AS_TTable[index].OnMove = ToMove;
      AS_TTable[index].Threat = threat;
    };
  
  return;
}

void LearnStoreTT(int score, unsigned nhash, unsigned hhash, int tomove, int best, int depth)
{
  unsigned long index;

  index = nhash % TTSIZE;

  AS_TTable[index].Depth = depth;
  AS_TTable[index].Type = EXACT;
  AS_TTable[index].Hash = nhash;
  AS_TTable[index].Hold_hash = hhash;
  AS_TTable[index].Bestmove = best;
  AS_TTable[index].Bound = score;
  AS_TTable[index].OnMove = tomove;
  AS_TTable[index].Threat = 0;

}

int ProbeTT(int *score, int alpha, int beta, int *best, int *threat, int *donull, int depth)
{

  unsigned long index;

  *donull = TRUE;

  TTProbes++;

  index = hash % TTSIZE;
  
  if ((DP_TTable[index].Hash == hash) 
      && (DP_TTable[index].Hold_hash == hold_hash) 
      && (DP_TTable[index].OnMove == ToMove))
    {
      TTHits++;
      
      /*if ((TTable[index].Type == UPPER) 
      	   && ((depth-2-1) <= TTable[index].Depth) 
      	   && (TTable[index].Bound < beta)) 
      	  *donull = FALSE;*/

      if (DP_TTable[index].Depth >= depth)
	{
	  *score = DP_TTable[index].Bound;
	  
	  if (*score > (+INF-500))
	   *score -= ply;
	  else if (*score < (-INF+500))
	    *score += ply;

	  *best = DP_TTable[index].Bestmove;
	  *threat = DP_TTable[index].Threat;

	  return DP_TTable[index].Type;
	}
      else
	{
	  *best = DP_TTable[index].Bestmove;
	  *threat = DP_TTable[index].Threat;

	  return DUMMY;
	}
    }
  else if ((AS_TTable[index].Hash == hash) 
      && (AS_TTable[index].Hold_hash == hold_hash) 
      && (AS_TTable[index].OnMove == ToMove))
    {
      TTHits++;

      if (AS_TTable[index].Depth >= depth)
	{
	  *score = AS_TTable[index].Bound;
	  
	  if (*score > (+INF-500))
	   *score -= ply;
	  else if (*score < (-INF+500))
	    *score += ply;

	  *best = AS_TTable[index].Bestmove;
	  *threat = AS_TTable[index].Threat;

	  return AS_TTable[index].Type;
	}
      else
	{
	  *best = AS_TTable[index].Bestmove;
	  *threat = AS_TTable[index].Threat;

	  return DUMMY;
	}
    }
  else
    return HMISS;

}






