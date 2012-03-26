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
    Purpose: functions for evaluating positions (standard chess)

*/

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"

/* these tables will be used for positional bonuses: */

static int sbishop[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-5,-5,-5,-5,-5,-5,-5,-5,0,0,
0,0,-5,10,5,10,10,5,10,-5,0,0,
0,0,-5,5,3,10,10,3,5,-5,0,0,
0,0,-5,3,10,6,6,10,3,-5,0,0,
0,0,-5,3,10,6,6,10,3,-5,0,0,
0,0,-5,5,3,10,10,3,5,-5,0,0,
0,0,-5,10,5,10,10,5,10,-5,0,0,
0,0,-5,-5,-5,-5,-5,-5,-5,-5,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

static int sknight[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,-10,0,0,3,3,0,0,-10,0,0,
0,0,-10,0,5,5,5,5,0,-10,0,0,
0,0,-10,0,5,10,10,5,0,-10,0,0,
0,0,-10,0,5,10,10,5,0,-10,0,0,
0,0,-10,0,5,5,5,5,0,-10,0,0,
0,0,-10,0,0,3,3,0,0,-10,0,0,
0,0,-20,-10,-10,-10,-10,-10,-10,-20,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

static long int swhite_pawn[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,1,2,3,10,10,3,2,1,0,0,
0,0,2,4,6,12,12,6,4,2,0,0,
0,0,3,6,9,14,14,9,6,3,0,0,
0,0,4,8,12,16,16,12,8,4,0,0,
0,0,5,10,15,20,20,15,10,5,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

static int sblack_pawn[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,5,10,15,20,20,15,10,5,0,0,
0,0,4,8,12,16,16,12,8,4,0,0,
0,0,3,6,9,14,14,9,6,3,0,0,
0,0,2,4,6,12,12,6,4,2,0,0,
0,0,1,2,3,10,10,3,2,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* to be used during opening and middlegame for white king positioning: */
static int swhite_king[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,2,14,0,0,0,9,14,2,0,0,
0,0,-3,-3,-5,-5,-5,-5,-3,-3,0,0,
0,0,-5,-5,-8,-8,-8,-8,-5,-5,0,0,
0,0,-8,-8,-13,-13,-13,-13,-8,-8,0,0,
0,0,-13,-13,-21,-21,-21,-21,-13,-13,0,0,
0,0,-21,-21,-34,-34,-34,-34,-21,-21,0,0,
0,0,-34,-34,-55,-55,-55,-55,-34,-34,0,0,
0,0,-55,-55,-89,-89,-89,-89,-55,-55,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* to be used during opening and middlegame for black king positioning: */
static int sblack_king[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-55,-55,-89,-89,-89,-89,-55,-55,0,0,
0,0,-34,-34,-55,-55,-55,-55,-34,-34,0,0,
0,0,-21,-21,-34,-34,-34,-34,-21,-21,0,0,
0,0,-13,-13,-21,-21,-21,-21,-13,-13,0,0,
0,0,-8,-8,-13,-13,-13,-13,-8,-8,0,0,
0,0,-5,-5,-8,-8,-8,-8,-5,-5,0,0,
0,0,-3,-3,-5,-5,-5,-5,-3,-3,0,0,
0,0,2,14,0,0,0,9,14,2,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* to be used for positioning of both kings during the endgame: */
static int send_king[144] = {
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,-5,-3,-1,0,0,-1,-3,-5,0,0,
0,0,-3,10,10,10,10,10,10,-3,0,0,
0,0,-1,10,25,25,25,25,10,-1,0,0,
0,0,0,10,25,30,30,25,10,0,0,0,
0,0,0,10,25,30,30,25,10,0,0,0,
0,0,-1,10,25,25,25,25,10,-1,0,0,
0,0,-3,10,10,10,10,10,-3,0,0,
0,0,-5,-3,-1,0,0,-1,-3,-5,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0};

/* utility array to reverse rank: */
static int srev_rank[9] = {
0,8,7,6,5,4,3,2,1};


long int end_eval (void) {

  /* return a score for the current endgame position: */

  int i, a, pawn_file, pawns[2][11], white_back_pawn[11], black_back_pawn[11],
    srank, j;
  long int score = 0;
  bool isolated, backwards;
  int in_cache;
  int wp = 0, bp = 0, wn = 0, bn = 0, wb = 0, bb = 0,
    wq = 0, bq = 0, wr = 0, br = 0;

  in_cache = 0;
  
  checkECache(&score, &in_cache);
  
  if(in_cache)
    {
      if (white_to_move == 1) return score;
      return -score;
    }


  /* initialize the pawns array, (files offset by one to use dummy files in
     order to easier determine isolated status) and also initialize the
     arrays keeping track of the rank of the most backward pawn: */
  memset (pawns, 0, sizeof (pawns));
  for (i = 0; i < 11; i++) {
    white_back_pawn[i] = 7;
    black_back_pawn[i] = 2;
  }
  for (j = 1, a = 1; (a <= piece_count); j++) {
     i = pieces[j];
    
    if (!i)
      continue;
    else
      a++;

    assert((i > 0) && (i < 145));
 
    pawn_file = file (i)+1;
    srank = rank (i);
    if (board[i] == wpawn) {
      pawns[1][pawn_file]++;
      if (srank < white_back_pawn[pawn_file]) {
	white_back_pawn[pawn_file] = srank;
      }
    }
    else if (board[i] == bpawn) {
      pawns[0][pawn_file]++;
      if (srank > black_back_pawn[pawn_file]) {
	black_back_pawn[pawn_file] = srank;
      }
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

    pawn_file = file (i)+1;
    srank = rank (i);
    switch (board[i]) {
      case (wpawn):
	isolated = FALSE;
	backwards = FALSE;
	score += 100;
	score += swhite_pawn[i];
	wp++;

	/* in general, bonuses/penalties in the endgame evaluation will be
	   higher, since pawn structure becomes more important for the
	   creation of passed pawns */

	/* check for backwards pawns: */
	if (white_back_pawn[pawn_file+1] > srank
	    && white_back_pawn[pawn_file-1] > srank) {
	  score -= 8;
	  backwards = TRUE;
	  /* check to see if it is furthermore isolated: */
	  if (!pawns[1][pawn_file+1] && !pawns[1][pawn_file-1]) {
	    score -= 5;
	    isolated = TRUE;
	  }
	}

	/* give weak, exposed pawns a penalty (not as much as in the midgame,
	   since there may be no pieces to take advantage of it): */
	if (!pawns[0][pawn_file]) {
	  if (backwards) score -= 3;
	  if (isolated) score -= 5;
	}

	/* give doubled, trippled, etc.. pawns a penalty (bigger in the
	   endgame, since they will become big targets): */
	if (pawns[1][pawn_file] > 1)
	  score -= 3*(pawns[1][pawn_file]-1);

	/* give bonuses for passed pawns (bigger in the endgame since passed
	   pawns are what wins the endgame): */
	if (!pawns[0][pawn_file] && srank >= black_back_pawn[pawn_file-1] &&
	    srank >= black_back_pawn[pawn_file+1]) {
	  score += 20 + 3*swhite_pawn[i];
	  /* give an extra bonus if a connected, passed pawn: */
	  if (!isolated)
	    score += 18;
	}

	break;

      case (bpawn):
	isolated = FALSE;
	backwards = FALSE;
	score -= 100;
	score -= sblack_pawn[i];
	bp++;

	/* in general, bonuses/penalties in the endgame evaluation will be
	   higher, since pawn structure becomes more important for the
	   creation of passed pawns */

	/* check for backwards pawns: */
	if (black_back_pawn[pawn_file+1] < srank
	    && black_back_pawn[pawn_file-1] < srank) {
	  score += 8;
	  backwards = TRUE;
	  /* check to see if it is furthermore isolated: */
	  if (!pawns[0][pawn_file+1] && !pawns[0][pawn_file-1]) {
	    score += 5;
	    isolated = TRUE;
	  }
	}

	/* give weak, exposed pawns a penalty (not as much as in the midgame,
	   since there may be no pieces to take advantage of it): */
	if (!pawns[1][pawn_file]) {
	  if (backwards) score += 3;
	  if (isolated) score += 5;
	}

	/* give doubled, trippled, etc.. pawns a penalty (bigger in the
	   endgame, since they will become big targets): */
	if (pawns[0][pawn_file] > 1)
	  score += 3*(pawns[0][pawn_file]-1);

	/* give bonuses for passed pawns (bigger in the endgame since passed
	   pawns are what wins the endgame): */
	if (!pawns[1][pawn_file] && srank <= white_back_pawn[pawn_file-1] &&
	    srank <= white_back_pawn[pawn_file+1]) {
	  score -= 20 + 3*sblack_pawn[i];
	  /* give an extra bonus if a connected, passed pawn: */
	  if (!isolated)
	    score -= 18;
	}

	break;

      case (wrook):
	score += 500;
	wr++;
	
	/* bonus for being on the 7th (a bit bigger bonus in the endgame, b/c
	   a rook on the 7th can be a killer in the endgame): */
	if (srank == 7)
	  score += 12;

	/* give bonuses depending on how open the rook's file is: */
	if (!pawns[1][pawn_file]) {
	  /* half open file */
	  score += 5;
	  if (!pawns[0][pawn_file]) {
	    /* open file */
	    score += 3;
	  }
	}

	break;

      case (brook):
	score -= 500;
	br++;

	/* bonus for being on the 7th (a bit bigger bonus in the endgame, b/c
	   a rook on the 7th can be a killer in the endgame): */
	if (srank == 2)
	  score -= 12;

	/* give bonuses depending on how open the rook's file is: */
	if (!pawns[0][pawn_file]) {
	  /* half open file */
	  score -= 5;
	  if (!pawns[1][pawn_file]) {
	    /* open file */
	    score -= 3;
	  }
	}

	break;

      case (wbishop):
	score += 325;
	score += sbishop[i];
	wb++;
	break;

      case (bbishop):
	score -= 325;
	score -= sbishop[i];
	bb++;
	break;

      case (wknight):
	score += 310;
	score += sknight[i];
	wn++;
	break;

      case (bknight):
	score -= 310;
	score -= sknight[i];
	bn++;
	break;

      case (wqueen):
	score += 900;
	wq++;
	break;

      case (bqueen):
	score -= 900;
	bq++;
	break;
    }
  }

  /* some static knowledge about drawn endgames */

 /* no more pawns */
  if (!wp && !bp)
    {
      /* nor heavies */
      if (!wr && !br && !wq && !bq)
	{
	  if (!bb && !wb)
	    {
	      /* only knights */
	      /* it pretty safe to say this is a draw */
	      if (wn < 3 && bn < 3)
		{
		  score = 0;
		}
	    }
	  else if (!wn && !bn)
	    {
	      /* only bishops */
	      /* not a draw if one side two other side zero
		 else its always a draw                     */
	      if (abs(wb - bb) < 2)
		{
		  score = 0;
		}
	    }
	  else if ((wn < 3 && !wb) || (wb == 1 && !wn))
	    {
	      /* we cant win, but can black? */
	      if ((bn < 3 && !bb) || (bb == 1 && !bn))
		{
		  /* guess not */
		  score = 0;
		}
	    }
	}
      else if (!wq && !bq)
	{
	  if (wr == 1 && br == 1)
	    {
	      /* rooks equal */
	      if ((wn + wb) < 2 && (bn + bb) < 2)	
		{
		  /* one minor difference max */
		  /* a draw too usually */
		  score = 0;
		}
	    }
	  else if (wr == 1 && !br)
	    {
	      /* one rook */
	      /* draw if no minors to support AND
		 minors to defend  */
	      if ((wn + wb == 0) && (((bn + bb) == 1) || ((bn + bb) == 2)))
		{
		  score = 0;
		}
	    }
	  else if (br == 1 && !wr)
	    {
	      /* one rook */
	      /* draw if no minors to support AND
		 minors to defend  */
	      if ((bn + bb == 0) && (((wn + wb) == 1) || ((wn + wb) == 2)))
		{
		  score = 0;
		}
	    }
	}
    }
  else
    {
      /* bad trade code, largely based on Crafty's */
      /* minors are not equal */
      if ((wn + wb) != (bn + bb))  
	{
	  /* majors are equal */
	  if ((wq + wr) == (bq + br))
	    {
	      if ((wn + wb) > (bn + bb))
		{
		  /* white is a piece up */
		  score += 120;
		}
	      else
		{
		  /* black is a piece up */
		  score -= 120;
		}
	    }
	  else if (abs((wr + wq) - (br + bq)) == 1)
	    {
	      /* one major difference */
	      
	      if ((wb + wn) > (bb + bn + 1))
		{
		  /* two minors for one major */
		  score += 120;
		}
	      else if ((bb + bn) > (wb + wn + 1))
		{
		  score -= 120;
		}
	    }
	  else if (abs((wr + wq) - (br + bq)) == 2)
	    {	
	      /* two majors difference */
	      
	      if ((wb + wn) > (bb + bn + 2))
		{
		  /* three minors for two majors */
		  score += 120;
		}
	      else if ((bb + bn) > (wb + wn + 2))
		{
		  score -= 120;
		}
	      
	    }
	}
      else if ((wq + wr) == (bq + br))
	{
	  if (wq && !bq) 
	    {
	      score += 120;
	    }
	  else if (!wq && bq)
	    {
	      score -= 120;
	    }
	}
    }
  
  /* the king is safe to come out in the endgame, so we don't check for
     king safety anymore, and encourage centralization of the king */
  score += send_king[wking_loc];
  
  /* the king is safe to come out in the endgame, so we don't check for
     king safety anymore, and encourage centralization of the king */
  score -= send_king[bking_loc];
  
  /* the e/d pawn blockage is not relevant in the endgame, and we don't need
     to check for king safety due to pawn storms / heavy piece infiltration */
  
  storeECache(score);

  /* adjust for color: */
  if (white_to_move == 1) {
    return score;
  }
  else {
    return -score;
  }

}

void check_phase(void)
{
  int num_pieces = 0;
  int j, a, i;

  for (j = 1, a = 1; (a <= piece_count); j++) 
    {
      i = pieces[j];
      
      if (!i)
	continue;
      else
	a++;
      
      if (board[i] != wpawn && board[i] != bpawn &&
	  board[i] != npiece && board[i] != frame) 
	{
	  num_pieces++;
	}
    };
  if ((num_pieces > 11) 
      /* not both have castled */
      && (!white_castled || !black_castled) 
      /* no both lost castling priveledges */
      && (board[30] == wking || board[114] == bking)) 
    {
      phase = Opening;
    }
  else if (num_pieces < 7) 
    {
      phase = Endgame;
    }
  else 
    phase = Middlegame;
  
}

long int seval (void) {

  /* select the appropriate eval() routine: */

  if (phase == Opening) {
    return (opn_eval ());
  }
  else if (phase == Endgame) {
    return (end_eval ());
  }
  else {
    return (mid_eval ());
  }

}


long int mid_eval (void) {

  /* return a score for the current middlegame position: */

  int i,a, pawn_file, pawns[2][11], white_back_pawn[11], black_back_pawn[11],
    srank, wking_pawn_file, bking_pawn_file, j;
  long int score = 0;
  bool isolated, backwards;
  int in_cache;
  int wp = 0, bp = 0, wn = 0, bn = 0, wb = 0, bb = 0,
	  wq = 0, bq = 0, wr = 0, br = 0;

  in_cache = 0;
  
  checkECache(&score, &in_cache);
  
  if(in_cache)
    {
      if (white_to_move == 1) return score;
      return -score;
    }

  /* initialize the pawns array, (files offset by one to use dummy files in
     order to easier determine isolated status) and also initialize the
     arrays keeping track of the rank of the most backward pawn: */
  memset (pawns, 0, sizeof (pawns));
  for (i = 0; i < 11; i++) {
    white_back_pawn[i] = 7;
    black_back_pawn[i] = 2;
  }
  for (j = 1, a = 1; (a <= piece_count); j++) {
    i = pieces[j];
    
    if (!i)
      continue;
    else
      a++;
 
    assert((i > 0) && (i < 145));
    
    pawn_file = file (i)+1;
    srank = rank (i);
    if (board[i] == wpawn) {
      pawns[1][pawn_file]++;
      if (srank < white_back_pawn[pawn_file]) {
	white_back_pawn[pawn_file] = srank;
      }
    }
    else if (board[i] == bpawn) {
      pawns[0][pawn_file]++;
      if (srank > black_back_pawn[pawn_file]) {
	black_back_pawn[pawn_file] = srank;
      }
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

    pawn_file = file (i)+1;
    srank = rank (i);
    switch (board[i]) {
      case (wpawn):
	isolated = FALSE;
	backwards = FALSE;
	score += 100;
	score += swhite_pawn[i];
	wp++;

	/* check for backwards pawns: */
	if (white_back_pawn[pawn_file+1] > srank
	    && white_back_pawn[pawn_file-1] > srank) {
	  score -= 5;
	  backwards = TRUE;
	  /* check to see if it is furthermore isolated: */
	  if (!pawns[1][pawn_file+1] && !pawns[1][pawn_file-1]) {
	    score -= 3;
	    isolated = TRUE;
	  }
	}

	/* give weak, exposed pawns a penalty: */
	if (!pawns[0][pawn_file]) {
	  if (backwards) score -= 4;
	  if (isolated) score -= 8;
	}

	/* give doubled, trippled, etc.. pawns a penalty: */
	if (pawns[1][pawn_file] > 1)
	  score -= 2*(pawns[1][pawn_file]-1);

	/* give bonuses for passed pawns: */
	if (!pawns[0][pawn_file] && srank >= black_back_pawn[pawn_file-1] &&
	    srank >= black_back_pawn[pawn_file+1]) {
	  score += 10 + 2*swhite_pawn[i];
	  /* give an extra bonus if a connected, passed pawn: */
	  if (!isolated)
	    score += 15;
	}

	break;

      case (bpawn):
	isolated = FALSE;
	backwards = FALSE;
	score -= 100;
	score -= sblack_pawn[i];
	bp++;

	/* check for backwards pawns: */
	if (black_back_pawn[pawn_file+1] < srank
	    && black_back_pawn[pawn_file-1] < srank) {
	  score += 5;
	  backwards = TRUE;
	  /* check to see if it is furthermore isolated: */
	  if (!pawns[0][pawn_file+1] && !pawns[0][pawn_file-1]) {
	    score += 3;
	    isolated = TRUE;
	  }
	}

	/* give weak, exposed pawns a penalty: */
	if (!pawns[1][pawn_file]) {
	  if (backwards) score += 4;
	  if (isolated) score += 8;
	}

	/* give doubled, trippled, etc.. pawns a penalty: */
	if (pawns[0][pawn_file] > 1)
	  score += 2*(pawns[0][pawn_file]-1);

	/* give bonuses for passed pawns: */
	if (!pawns[1][pawn_file] && srank <= white_back_pawn[pawn_file-1] &&
	    srank <= white_back_pawn[pawn_file+1]) {
	  score -= 10 + 2*sblack_pawn[i];
	  /* give an extra bonus if a connected, passed pawn: */
	  if (!isolated)
	    score -= 15;
	}

	break;

      case (wrook):
	score += 500;
	wr++;
	
	/* bonus for being on the 7th: */
	if (srank == 7)
	  score += 8;

	/* give bonuses depending on how open the rook's file is: */
	if (!pawns[1][pawn_file]) {
	  /* half open file */
	  score += 5;
	  if (!pawns[0][pawn_file]) {
	    /* open file */
	    score += 3;
	  }
	}

	break;

      case (brook):
	score -= 500;
	br++;

	/* bonus for being on the 7th: */
	if (srank == 2)
	  score -= 8;

	/* give bonuses depending on how open the rook's file is: */
	if (!pawns[0][pawn_file]) {
	  /* half open file */
	  score -= 5;
	  if (!pawns[1][pawn_file]) {
	    /* open file */
	    score -= 3;
	  }
	}

	break;

      case (wbishop):
	score += 325;
	score += sbishop[i];
	wb++;
	break;

      case (bbishop):
	score -= 325;
	score -= sbishop[i];
	bb++;
	break;

      case (wknight):
	score += 310;
	score += sknight[i];
	wn++;
	break;

      case (bknight):
	score -= 310;
	score -= sknight[i];
	bn++;
	break;

      case (wqueen):
	score += 900;
	wq++;
	break;

      case (bqueen):
	score -= 900;
	bq++;
	break;

      case (wking):
	score += swhite_king[i];

	/* encourage castling, and give a penalty for moving the king without
	   castling */
	if (white_castled)
	  score += 20;
	else if (moved[30]) {
	  score -= 7;
	  /* make the penalty bigger if the king is open, leaving the other
	     side a chance to gain tempo with files along the file, as well
	     as building an attack: */
	  if (!pawns[1][pawn_file])
	    score -= 8;
	}

	/* if the king is behind some pawn cover, give penalties for the pawn
	   cover being far from the king, else give a penalty for the king
	   not having any pawn cover: */
	if (srank < white_back_pawn[pawn_file] && pawns[1][pawn_file])
	  score -= 8*(white_back_pawn[pawn_file]-srank-1);
	else
	  score -= 16;
	if (srank < white_back_pawn[pawn_file+1] && pawns[1][pawn_file+1])
	  score -= 8*(white_back_pawn[pawn_file+1]-srank-1);
	else
	  score -= 16;
	if (srank < white_back_pawn[pawn_file-1] && pawns[1][pawn_file-1])
	  score -= 8*(white_back_pawn[pawn_file-1]-srank-1);
	else
	  score -= 16;	  

	break;

      case (bking):
	score -= sblack_king[i];

	/* encourage castling, and give a penalty for moving the king without
	   castling */
	if (black_castled)
	  score -= 20;
	else if (moved[114]) {
	  score += 7;
	  /* make the penalty bigger if the king is open, leaving the other
	     side a chance to gain tempo with files along the file, as well
	     as building an attack: */
	  if (!pawns[0][pawn_file])
	    score += 8;
	}

	/* if the king is behind some pawn cover, give penalties for the pawn
	   cover being far from the king, else give a penalty for the king
	   not having any pawn cover: */
	if (srank > black_back_pawn[pawn_file] && pawns[0][pawn_file])
	  score += 8*(srev_rank[srank-black_back_pawn[pawn_file]-1]);
	else
	  score += 16;
	if (srank > black_back_pawn[pawn_file+1] && pawns[0][pawn_file+1])
	  score += 8*(srev_rank[srank-black_back_pawn[pawn_file+1]-1]);
	else
	  score += 16;
	if (srank > black_back_pawn[pawn_file-1] && pawns[0][pawn_file-1])
	  score += 8*(srev_rank[srank-black_back_pawn[pawn_file-1]-1]);
	else
	  score += 16;

	break;
    }
  }

  /* give penalties for blocking the e/d pawns: */
  if (!moved[41] && board[53] != npiece)
    score -= 5;
  if (!moved[42] && board[54] != npiece)
    score -= 5;
  if (!moved[101] && board[89] != npiece)
    score += 5;
  if (!moved[102] && board[90] != npiece)
    score += 5;

  /* to be used for pawn storm code: */
  wking_pawn_file = file (wking_loc)+1;
  bking_pawn_file = file (bking_loc)+1;

  /* if the kings are on opposite wings, or far apart, check for pawn
     storms, and open lines for heavy pieces: */
  if ((wking_pawn_file-bking_pawn_file) > 2 ||
      (bking_pawn_file-wking_pawn_file) > 2) {
    /* black pawn storms: */
    score -= 3*(srev_rank[black_back_pawn[wking_pawn_file]]-2);
    score -= 3*(srev_rank[black_back_pawn[wking_pawn_file+1]]-2);
    score -= 3*(srev_rank[black_back_pawn[wking_pawn_file-1]]-2);

    /* white pawn storms: */
    score += 3*(white_back_pawn[bking_pawn_file]-2);
    score += 3*(white_back_pawn[bking_pawn_file+1]-2);
    score += 3*(white_back_pawn[bking_pawn_file-1]-2);

    /* black opening up lines: */
    if (!pawns[0][wking_pawn_file])
      score -= 8;
    if (!pawns[0][wking_pawn_file+1])
      score -= 6;
    if (!pawns[0][wking_pawn_file-1])
      score -= 6;

    /* white opening up lines: */
    if (!pawns[1][bking_pawn_file])
      score += 8;
    if (!pawns[1][bking_pawn_file+1])
      score += 6;
    if (!pawns[1][bking_pawn_file-1])
      score += 6;

  }

  /* bad trade code, largely based on Crafty's */

  /* minors are not equal */
  if ((wn + wb) != (bn + bb))  
    {
      /* majors are equal */
      if ((wq + wr) == (bq + br))
	{
	  if ((wn + wb) > (bn + bb))
	    {
	      /* white is a piece up */
	      score += 120;
	    }
	  else
	    {
	      /* black is a piece up */
	      score -= 120;
	    }
	}
      else if (abs((wr + wq) - (br + bq)) == 1)
	{
	  /* one major difference */
	  
	  if ((wb + wn) > (bb + bn + 1))
	    {
	      /* two minors for one major */
	      score += 120;
	    }
	  else if ((bb + bn) > (wb + wn + 1))
	    {
	      score -= 120;
	    }
	}
      else if (abs((wr + wq) - (br + bq)) == 2)
	{	
	  /* two majors difference */
	  
	  if ((wb + wn) > (bb + bn + 2))
	    {
	      /* three minors for two majors */
	      score += 120;
	    }
	  else if ((bb + bn) > (wb + wn + 2))
	    {
	      score -= 120;
	    }
	  
	}
    }
  else if ((wq + wr) == (bq + br))
    {
      if (wq && !bq) 
	{
	  score += 120;
	}
      else if (!wq && bq)
	{
	  score -= 120;
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

long int opn_eval (void) {

  /* return a score for the current opening position: */

  int i,a, pawn_file, pawns[2][11], white_back_pawn[11], black_back_pawn[11],
    srank, wking_pawn_file, bking_pawn_file, j;
  long int score = 0;
  bool isolated, backwards;
  int in_cache;

  in_cache = 0;
  
  checkECache(&score, &in_cache);
  
  if(in_cache)
    {
      if (white_to_move == 1) return score;
      return -score;
    }

  /* initialize the pawns array, (files offset by one to use dummy files in
     order to easier determine isolated status) and also initialize the
     arrays keeping track of the rank of the most backward pawn: */
  memset (pawns, 0, sizeof (pawns));
  for (i = 0; i < 11; i++) {
    white_back_pawn[i] = 7;
    black_back_pawn[i] = 2;
  }
 for (j = 1, a = 1; (a <= piece_count); j++) {
     i = pieces[j];
    
    if (!i)
      continue;
    else
      a++;

    pawn_file = file (i)+1;
    srank = rank (i);
    if (board[i] == wpawn) {
      pawns[1][pawn_file]++;
      if (srank < white_back_pawn[pawn_file]) {
	white_back_pawn[pawn_file] = srank;
      }
    }
    else if (board[i] == bpawn) {
      pawns[0][pawn_file]++;
      if (srank > black_back_pawn[pawn_file]) {
	black_back_pawn[pawn_file] = srank;
      }
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

    assert((i > 0) && (i < 145));

    pawn_file = file (i)+1;
    srank = rank (i);
    switch (board[i]) {
      case (wpawn):
	isolated = FALSE;
	backwards = FALSE;
	score += 100;
	score += swhite_pawn[i];

	/* penalties / bonuses will be in general smaller in the opening,
	   in order to put an emphasis on piece development */

	/* check for backwards pawns: */
	if (white_back_pawn[pawn_file+1] > srank
	    && white_back_pawn[pawn_file-1] > srank) {
	  /* no penalty in the opening for having a backwards pawn that hasn't
	     moved yet! */
	  if (srank != 2)
	    score -= 3;
	  backwards = TRUE;
	  /* check to see if it is furthermore isolated: */
	  if (!pawns[1][pawn_file+1] && !pawns[1][pawn_file-1]) {
	    score -= 2;
	    isolated = TRUE;
	  }
	}

	/* give weak, exposed pawns a penalty: */
	if (!pawns[0][pawn_file]) {
	  if (backwards) score -= 3;
	  if (isolated) score -= 5;
	}

	/* give doubled, trippled, etc.. pawns a penalty: */
	if (pawns[1][pawn_file] > 1)
	  score -= 2*(pawns[1][pawn_file]-1);

	/* give bonuses for passed pawns: */
	if (!pawns[0][pawn_file] && srank >= black_back_pawn[pawn_file-1] &&
	    srank >= black_back_pawn[pawn_file+1]) {
	  score += 5 + 2*swhite_pawn[i];
	  /* give an extra bonus if a connected, passed pawn: */
	  if (!isolated)
	    score += 10;
	}

	break;

      case (bpawn):
	isolated = FALSE;
	backwards = FALSE;
	score -= 100;
	score -= sblack_pawn[i];

	/* penalties / bonuses will be in general smaller in the opening,
	   in order to put an emphasis on piece development */

	/* check for backwards pawns: */
	if (black_back_pawn[pawn_file+1] < srank
	    && black_back_pawn[pawn_file-1] < srank) {
	  /* no penalty in the opening for having a backwards pawn that hasn't
	     moved yet! */
	  if (srank != 2)
	    score += 3;
	  backwards = TRUE;
	  /* check to see if it is furthermore isolated: */
	  if (!pawns[0][pawn_file+1] && !pawns[0][pawn_file-1]) {
	    score += 2;
	    isolated = TRUE;
	  }
	}

	/* give weak, exposed pawns a penalty: */
	if (!pawns[1][pawn_file]) {
	  if (backwards) score += 3;
	  if (isolated) score += 5;
	}

	/* give doubled, trippled, etc.. pawns a penalty: */
	if (pawns[0][pawn_file] > 1)
	  score += 2*(pawns[0][pawn_file]-1);

	/* give bonuses for passed pawns: */
	if (!pawns[1][pawn_file] && srank <= white_back_pawn[pawn_file-1] &&
	    srank <= white_back_pawn[pawn_file+1]) {
	  score -= 5 + 2*sblack_pawn[i];
	  /* give an extra bonus if a connected, passed pawn: */
	  if (!isolated)
	    score -= 10;
	}

	break;

      case (wrook):
	score += 500;
	
	/* bonus for being on the 7th: */
	if (srank == 7)
	  score += 8;

	/* give bonuses depending on how open the rook's file is: */
	if (!pawns[1][pawn_file]) {
	  /* half open file */
	  score += 5;
	  if (!pawns[0][pawn_file]) {
	    /* open file */
	    score += 3;
	  }
	}

	break;

      case (brook):
	score -= 500;

	/* bonus for being on the 7th: */
	if (srank == 2)
	  score -= 8;

	/* give bonuses depending on how open the rook's file is: */
	if (!pawns[0][pawn_file]) {
	  /* half open file */
	  score -= 5;
	  if (!pawns[1][pawn_file]) {
	    /* open file */
	    score -= 3;
	  }
	}

	break;

      case (wbishop):
	score += 325;
	score += sbishop[i];
	break;

      case (bbishop):
	score -= 325;
	score -= sbishop[i];
	break;

      case (wknight):
	score += 310;
	score += sknight[i];
	break;

      case (bknight):
	score -= 310;
	score -= sknight[i];
	break;

      case (wqueen):
	score += 900;

	/* a small penalty to discourage moving the queen in the opening
	   before the other minors: */
	if (i != 29)
	  if (!moved[28] || !moved[27] || !moved[31] || !moved[32])
	    score -= 4;

	break;

      case (bqueen):
	score -= 900;

	/* a small penalty to discourage moving the queen in the opening
	   before the other minors: */
	if (i != 113)
	  if (!moved[112] || !moved[111] || !moved[115] || !moved[116])
	    score += 4;

	break;

      case (wking):
	score += swhite_king[i];

	/* encourage castling, and give a penalty for moving the king without
	   castling */
	if (white_castled)
	  score += 20;
	else if (moved[30]) {
	  score -= 15;
	  /* make the penalty bigger if the king is open, leaving the other
	     side a chance to gain tempo with files along the file, as well
	     as building an attack: */
	  if (!pawns[1][pawn_file])
	    score -= 8;
	}

	/* in general, in the opening, don't worry quite so much about pawn
	   cover, because sometimes it isn't good for the king to castle */

	/* if the king is behind some pawn cover, give penalties for the pawn
	   cover being far from the king, else give a penalty for the king
	   not having any pawn cover: */
	if (srank < white_back_pawn[pawn_file] && pawns[1][pawn_file])
	  score -= 5*(white_back_pawn[pawn_file]-srank-1);
	else
	  score -= 8;
	if (srank < white_back_pawn[pawn_file+1] && pawns[1][pawn_file+1])
	  score -= 4*(white_back_pawn[pawn_file+1]-srank-1);
	else
	  score -= 8;
	if (srank < white_back_pawn[pawn_file-1] && pawns[1][pawn_file-1])
	  score -= 4*(white_back_pawn[pawn_file-1]-srank-1);
	else
	  score -= 8;	  

	break;

      case (bking):
	score -= sblack_king[i];

	/* encourage castling, and give a penalty for moving the king without
	   castling */
	if (black_castled)
	  score -= 20;
	else if (moved[114]) {
	  score += 15;
	  /* make the penalty bigger if the king is open, leaving the other
	     side a chance to gain tempo with files along the file, as well
	     as building an attack: */
	  if (!pawns[0][pawn_file])
	    score += 8;
	}

	/* in general, in the opening, don't worry quite so much about pawn
	   cover, because sometimes it isn't good for the king to castle */

	/* if the king is behind some pawn cover, give penalties for the pawn
	   cover being far from the king, else give a penalty for the king
	   not having any pawn cover: */
	if (srank > black_back_pawn[pawn_file] && pawns[0][pawn_file])
	  score += 5*(srev_rank[srank-black_back_pawn[pawn_file]-1]);
	else
	  score += 8;
	if (srank > black_back_pawn[pawn_file+1] && pawns[0][pawn_file+1])
	  score += 4*(srev_rank[srank-black_back_pawn[pawn_file+1]-1]);
	else
	  score += 8;
	if (srank > black_back_pawn[pawn_file-1] && pawns[0][pawn_file-1])
	  score += 4*(srev_rank[srank-black_back_pawn[pawn_file-1]-1]);
	else
	  score += 8;

	break;
    }
  }

  /* give bigger penalties for blocking the e/d pawns in the opening, as
     we want to develop quickly: */
  if (!moved[41] && board[53] != npiece)
    score -= 7;
  if (!moved[42] && board[54] != npiece)
    score -= 7;
  if (!moved[101] && board[89] != npiece)
    score += 7;
  if (!moved[102] && board[90] != npiece)
    score += 7;

  /* to be used for pawn storm code: */
  wking_pawn_file = file[wking_loc]+1;
  bking_pawn_file = file[bking_loc]+1;

  /* if the kings are on opposite wings, or far apart, check for pawn
     storms, and open lines for heavy pieces (bonuses/penalties brought
     down a bit in the opening, as it isn't a good idea to start pawn
     storming when the position is still fluid): */
  if ((wking_pawn_file-bking_pawn_file) > 2 ||
      (bking_pawn_file-wking_pawn_file) > 2) {
    /* black pawn storms: */
    score -= srev_rank[black_back_pawn[wking_pawn_file]] - 2;
    score -= srev_rank[black_back_pawn[wking_pawn_file+1]] - 2;
    score -= srev_rank[black_back_pawn[wking_pawn_file-1]] - 2;

    /* white pawn storms: */
    score += white_back_pawn[bking_pawn_file] - 2;
    score += white_back_pawn[bking_pawn_file+1] - 2;
    score += white_back_pawn[bking_pawn_file-1] - 2;

    /* black opening up lines: */
    if (!pawns[0][wking_pawn_file])
      score -= 6;
    if (!pawns[0][wking_pawn_file+1])
      score -= 4;
    if (!pawns[0][wking_pawn_file-1])
      score -= 4;

    /* white opening up lines: */
    if (!pawns[1][bking_pawn_file])
      score += 6;
    if (!pawns[1][bking_pawn_file+1])
      score += 4;
    if (!pawns[1][bking_pawn_file-1])
      score += 4;

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
