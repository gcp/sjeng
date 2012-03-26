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

    File: learn.c                                        
    Purpose: remembering of positions

*/

#include "sjeng.h"
#include "protos.h"
#include "extvars.h"

typedef struct 
{
  signed Depth:7;
  unsigned OnMove:1;  
  unsigned char Bestmove;
  unsigned long Hash;
  unsigned long Hold_hash;
  signed long Bound;
}
LearnType;

void Learn(int score, int best, int depth)
{
  int number = 0, next = 0;
  LearnType draft;

  printf("Learning score: %d  best: %d  depth:%d  hash: %X\n", score, best, depth, hash);
  
  if (Variant == Normal)
    {
      fseek(lrn_standard, 0, SEEK_SET);
      fread(&number, sizeof(int), 1, lrn_standard);
      fread(&next, sizeof(int), 1, lrn_standard);
      
      if (number < 50000) number++;

      fseek(lrn_standard, 0, SEEK_SET);
      fwrite(&number, sizeof(int), 1, lrn_standard);
      
      next++;
      if (next == 50000) next = 1;

      fwrite(&next, sizeof(int), 1, lrn_standard);

      fseek(lrn_standard, (2*sizeof(int)) + ((next-1)*sizeof(LearnType)), SEEK_SET);

      draft.Depth = depth;
      draft.OnMove = ToMove;
      draft.Hash = hash;
      draft.Hold_hash = hold_hash;
      draft.Bound = score;
      draft.Bestmove = best;

      fwrite(&draft, sizeof(draft), 1, lrn_standard);

      fflush(lrn_standard);
    }
  else
    {
      fseek(lrn_zh, 0, SEEK_SET);
      fread(&number, sizeof(int), 1, lrn_zh);
      fread(&next, sizeof(int), 1, lrn_zh);
      
      if (number < 50000) number++;

      fseek(lrn_zh, 0, SEEK_SET);
      fwrite(&number, sizeof(int), 1, lrn_zh);
      
      next++;
      if (next == 50000) next = 1;

      fwrite(&next, sizeof(int), 1, lrn_zh);

      fseek(lrn_zh, (2*sizeof(int)) + ((next-1)*sizeof(LearnType)), SEEK_SET);

      draft.Depth = depth;
      draft.OnMove = ToMove;
      draft.Hash = hash;
      draft.Hold_hash = hold_hash;
      draft.Bound = score;
      draft.Bestmove = best;

      fwrite(&draft, sizeof(LearnType), 1, lrn_zh);

      fflush(lrn_zh);
    }
}

void LoadLearn(void)
{
  int number = 0, posloop;
  LearnType draft;
    
  if (((Variant == Crazyhouse) || (Variant == Bughouse)) && (!lrn_zh))
    return;
  else if ((Variant == Normal) && !lrn_standard)
    return;
  
  if (Variant == Normal)
    {
      fseek(lrn_standard, 0, SEEK_SET);
      fread(&number, sizeof(int), 1, lrn_standard);
      fseek(lrn_standard, 2*sizeof(int), SEEK_SET);

      for (posloop = 0; posloop < number; posloop++)
	{
	  fread(&draft, sizeof(LearnType), 1, lrn_standard);

	  /*  	 printf("Learnstore: bd: %d  h: %X  hh: %X  om: %d  bm: %d d: %d\n",
	  	 draft.Bound, draft.Hash, draft.Hold_hash, draft.OnMove,
	  	 draft.Bestmove, draft.Depth);*/

	  LearnStoreTT(draft.Bound, draft.Hash, draft.Hold_hash, 
		       draft.OnMove, draft.Bestmove, draft.Depth);	  
	}
    }
  else
    {
      fseek(lrn_zh, 0, SEEK_SET);
      fread(&number, sizeof(int), 1, lrn_zh);
      fseek(lrn_zh, 2*sizeof(int), SEEK_SET);

      for (posloop = 0; posloop < number; posloop++)
	{
	  fread(&draft, sizeof(LearnType), 1, lrn_zh);
	  
	  LearnStoreTT(draft.Bound, draft.Hash, draft.Hold_hash,
		       draft.OnMove, draft.Bestmove, draft.Depth);	  
	}
    }

  return;
}
