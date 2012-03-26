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

    File: moves.c                                        
    Purpose: functions used to generate & make moves

*/

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"

unsigned long total_moves;
unsigned long total_movegens;

bool check_legal (move_s moves[], int m) {

  /* determines if a move made was legal.  Checks to see if the player who
     just moved castled through check, or is in check.  If the move made
     was illegal, returns FALSE, otherwise, returns TRUE. */

  int castled = moves[m].castled;

  /* check for castling moves: */
  if (castled) {
    /* white kingside castling: */
    if (castled == wck) {
      if (is_attacked (30, 0)) return FALSE;
      if (is_attacked (31, 0)) return FALSE;
      if (is_attacked (32, 0)) return FALSE;
      return TRUE;
    }
    /* white queenside castling: */
    if (castled == wcq) {
      if (is_attacked (30, 0)) return FALSE;
      if (is_attacked (29, 0)) return FALSE;
      if (is_attacked (28, 0)) return FALSE;
      return TRUE;
    }
    /* black kingside castling: */
    if (castled == bck) {
      if (is_attacked (114, 1)) return FALSE;
      if (is_attacked (115, 1)) return FALSE;
      if (is_attacked (116, 1)) return FALSE;
      return TRUE;
    }
    /* black queenside castling: */
    if (castled == bcq) {
      if (is_attacked (114, 1)) return FALSE;
      if (is_attacked (113, 1)) return FALSE;
      if (is_attacked (112, 1)) return FALSE;
      return TRUE;
    }
  }

  /* otherwise, just check on the kings: */
  /* black king: */
  else if (white_to_move%2) {
    if (is_attacked (bking_loc, 1)) return FALSE;
    else return TRUE;
  }

  /* white king: */
  else {
    if (is_attacked (wking_loc, 0)) return FALSE;
    else return TRUE;
  }

  /* should never get here .. but just so it will compile :P */
  return FALSE;

}


void gen (move_s moves[], int *num_moves) {

  /* generate pseudo-legal moves, and place them in the moves array */

  int from;

  /* generate white moves, if it is white to move: */
  if (white_to_move) {
    for (from = (26) ; from < (118) ; from++)  {
       switch (board[from]) {
       case (frame): /*from += */
	 from += 3;
	 continue;
       case (wpawn):
	 /* pawn moving up two squares on its first move: */
	 if (rank(from) == 2 && board[from+24] == npiece
	     && board[from+12] == npiece && !captures)
	   push_pawn (&moves[0], num_moves, from, from+24, FALSE);
	 /* pawn capturing diagonally: */
	 if (board[from+13]%2 == 0 && board[from+13] != frame)
	   push_pawn (&moves[0], num_moves, from, from+13, FALSE);
	 /* pawn captruing diagonally: */
	 if (board[from+11]%2 == 0 && board[from+11] != frame)
	   push_pawn (&moves[0], num_moves, from, from+11, FALSE);
	 /* ep move: */
	 if (ep_square == from+13)
	   push_pawn (&moves[0], num_moves, from, from+13, TRUE);
	 /* ep move: */
	 if (ep_square == from+11)
	   push_pawn (&moves[0], num_moves, from, from+11, TRUE);
	 /* pawn moves up one square: */
	 if (board[from+12] == npiece) {
	   /* only promotions when captures == TRUE */
	   if (rank (from) == 7) {
	     push_pawn (&moves[0], num_moves, from, from+12, FALSE);
	   }
	   else if (!captures) {
	     push_pawn (&moves[0], num_moves, from, from+12, FALSE);
	   }
	 }
	 break;
       case (wknight):
	 /* use the knight offsets: */
	 push_knight (&moves[0], num_moves, from, from+10);
	 push_knight (&moves[0], num_moves, from, from-10);
	 push_knight (&moves[0], num_moves, from, from+14);
	 push_knight (&moves[0], num_moves, from, from-14);
	 push_knight (&moves[0], num_moves, from, from+23);
	 push_knight (&moves[0], num_moves, from, from-23);
	 push_knight (&moves[0], num_moves, from, from+25);
	 push_knight (&moves[0], num_moves, from, from-25);
	 break;
       case (wbishop):
	 /* use the bishop offsets: */
	 push_slide (&moves[0], num_moves, from, from+13);
	 push_slide (&moves[0], num_moves, from, from-13);
	 push_slide (&moves[0], num_moves, from, from+11);
	 push_slide (&moves[0], num_moves, from, from-11);
	 break;
       case (wrook):
	 /* use the rook offsets: */
	 push_slide (&moves[0], num_moves, from, from+12);
	 push_slide (&moves[0], num_moves, from, from-12);
	 push_slide (&moves[0], num_moves, from, from+1);
	 push_slide (&moves[0], num_moves, from, from-1);
	 break;
       case (wqueen):
	 /* use the queen offsets: */
	 push_slide (&moves[0], num_moves, from, from+13);
	 push_slide (&moves[0], num_moves, from, from-13);
	 push_slide (&moves[0], num_moves, from, from+11);
	 push_slide (&moves[0], num_moves, from, from-11);
	 push_slide (&moves[0], num_moves, from, from+12);
	 push_slide (&moves[0], num_moves, from, from-12);
	 push_slide (&moves[0], num_moves, from, from+1);
	 push_slide (&moves[0], num_moves, from, from-1);
	 break;
       case (wking):
	 /* use the king offsets for 'normal' moves: */
	  push_king (&moves[0], num_moves, from, from+13, no_castle);
	  push_king (&moves[0], num_moves, from, from-13, no_castle);
	  push_king (&moves[0], num_moves, from, from+11, no_castle);
	  push_king (&moves[0], num_moves, from, from-11, no_castle);
	  push_king (&moves[0], num_moves, from, from+12, no_castle);
	  push_king (&moves[0], num_moves, from, from-12, no_castle);
	  push_king (&moves[0], num_moves, from, from+1, no_castle);
	  push_king (&moves[0], num_moves, from, from-1, no_castle);
	  /* castling moves: */
	  if (from == 30 && !moved[30] && !captures) {
	    /* kingside: */
	    if (!moved[33] && board[33] == wrook)
	      if (board[31] == npiece && board[32] == npiece)
		push_king (&moves[0], num_moves, from, from+2, wck);
	    /* queenside: */
	    if (!moved[26] && board[26] == wrook)
	      if (board[27] == npiece && board[28] == npiece
		  && board[29] == npiece)
		push_king (&moves[0], num_moves, from, from-2, wcq);
	  }
	  break;
       case (npiece):
	 if (Variant != Normal)
	   {
	     if (!captures)
	       {
		 if(holding[WHITE][wpawn])
		   {  
		     if ((rank(from) != 8) && (rank(from) != 1))	
		       {
			 try_drop(&moves[0], num_moves, wpawn, from);
		       }
		   }   
		 if(holding[WHITE][wknight])
		   {
		     try_drop(&moves[0], num_moves, wknight, from);
		   }
		 if(holding[WHITE][wbishop])
		   {
		     try_drop(&moves[0], num_moves, wbishop, from);
		   }
		 if(holding[WHITE][wrook])
		   {
		     try_drop(&moves[0], num_moves, wrook, from);
		   }	
		 if(holding[WHITE][wqueen])
		   {
		     try_drop(&moves[0], num_moves, wqueen, from);
		   }
	       };
	   }
        default:
	  break;
      }
    }
  }

  /* generate black moves, if it is black to move: */
  else {
    for (from = (26) ; from < (118) ; from++) {
      switch (board[from]) {
      case (frame):
	from += 3;
	continue;
      case (bpawn):
	/* pawn moving up two squares on its first move: */
	if (rank(from) == 7 && board[from-24] == npiece
	    && board[from-12] == npiece && !captures)
	  push_pawn (&moves[0], num_moves, from, from-24, FALSE);
	/* pawn capturing diagonally: */
	if (board[from-13]%2 == 1 && board[from-13] != npiece)
	  push_pawn (&moves[0], num_moves, from, from-13, FALSE);
	/* pawn capturing diagonally: */
	if (board[from-11]%2 == 1 && board[from-11] != npiece)
	  push_pawn (&moves[0], num_moves, from, from-11, FALSE);
	/* ep move: */
	if (ep_square == from-13)
	  push_pawn (&moves[0], num_moves, from, from-13, TRUE);
	/* ep move: */
	if (ep_square == from-11)
	  push_pawn (&moves[0], num_moves, from, from-11, TRUE);
	/* pawn moves up one square: */
	if (board[from-12] == npiece) {
	  /* only promotions when captures == TRUE */
	  if (rank (from) == 2) {
	    push_pawn (&moves[0], num_moves, from, from-12, FALSE);
	  }
	  else if (!captures) {
	    push_pawn (&moves[0], num_moves, from, from-12, FALSE);
	  }
	}
	  break;
      case (bknight):
	/* use the knight offsets: */
	push_knight (&moves[0], num_moves, from, from+10);
	push_knight (&moves[0], num_moves, from, from-10);
	push_knight (&moves[0], num_moves, from, from+14);
	push_knight (&moves[0], num_moves, from, from-14);
	push_knight (&moves[0], num_moves, from, from+23);
	push_knight (&moves[0], num_moves, from, from-23);
	push_knight (&moves[0], num_moves, from, from+25);
	push_knight (&moves[0], num_moves, from, from-25);
	  break;
      case (bbishop):
	/* use the bishop offsets: */
	push_slide (&moves[0], num_moves, from, from+13);
	push_slide (&moves[0], num_moves, from, from-13);
	push_slide (&moves[0], num_moves, from, from+11);
	push_slide (&moves[0], num_moves, from, from-11);
	break;
      case (brook):
	/* use the rook offsets: */
	push_slide (&moves[0], num_moves, from, from+12);
	push_slide (&moves[0], num_moves, from, from-12);
	push_slide (&moves[0], num_moves, from, from+1);
	push_slide (&moves[0], num_moves, from, from-1);
	break;
      case (bqueen):
	/* use the queen offsets: */
	push_slide (&moves[0], num_moves, from, from+13);
	push_slide (&moves[0], num_moves, from, from-13);
	push_slide (&moves[0], num_moves, from, from+11);
	push_slide (&moves[0], num_moves, from, from-11);
	push_slide (&moves[0], num_moves, from, from+12);
	push_slide (&moves[0], num_moves, from, from-12);
	push_slide (&moves[0], num_moves, from, from+1);
	push_slide (&moves[0], num_moves, from, from-1);
	break;
      case (bking):
	  /* use the king offsets for 'normal' moves: */
	push_king (&moves[0], num_moves, from, from+13, no_castle);
	push_king (&moves[0], num_moves, from, from-13, no_castle);
	push_king (&moves[0], num_moves, from, from+11, no_castle);
	push_king (&moves[0], num_moves, from, from-11, no_castle);
	push_king (&moves[0], num_moves, from, from+12, no_castle);
	push_king (&moves[0], num_moves, from, from-12, no_castle);
	push_king (&moves[0], num_moves, from, from+1, no_castle);
	push_king (&moves[0], num_moves, from, from-1, no_castle);
	/* castling moves: */
	if (from == 114 && !moved[114] && !captures) {
	  /* kingside: */
	  if (!moved[117] && board[117] == brook)
	    if (board[115] == npiece && board[116] == npiece)
	      push_king (&moves[0], num_moves, from, from+2, bck);
	  /* queenside: */
	  if (!moved[110] && board[110] == brook)
	    if (board[111] == npiece && board[112] == npiece
		&& board[113] == npiece)
	      push_king (&moves[0], num_moves, from, from-2, bcq);
	}
	break;
      case (npiece):
	if (Variant != Normal)
	  {
	    if (!captures)
	      {
		if(holding[BLACK][bpawn])
		  {  
		    if ((rank(from) != 8) && (rank(from) != 1))	
		      {
			try_drop(&moves[0], num_moves, bpawn, from);
		      }
		  }   
		if(holding[BLACK][bknight])
		  {
		    try_drop(&moves[0], num_moves, bknight, from);
		  }
		if(holding[BLACK][bbishop])
		  {
		    try_drop(&moves[0], num_moves, bbishop, from);
		  }
		if(holding[BLACK][brook])
		  {
		    try_drop(&moves[0], num_moves, brook, from);
		  }	
		if(holding[BLACK][bqueen])
		  {
		    try_drop(&moves[0], num_moves, bqueen, from);
		  }
	      };
	  };
      default:
	break;
      }
    }
  }

#ifdef XSTAT
 if (!captures)
   {
     total_moves += *num_moves;
     total_movegens++;
   };
#endif

}


bool in_check (void) {
  
  /* return true if the side to move is in check: */
  
  if (white_to_move == 1) {
    if (is_attacked (wking_loc, 0)) {
      return TRUE;
    }
  }
  else {
    if (is_attacked (bking_loc, 1)) {
      return TRUE;
    }
  }

  return FALSE;

}


bool is_attacked (int square, int color) {

  /* this function will return TRUE if square "square" is attacked by a piece
     of color "color", and return FALSE otherwise */

  static const int rook_o[4] = {12, -12, 1, -1};
  static const int bishop_o[4] = {11, -11, 13, -13};
  static const int knight_o[8] = {10, -10, 14, -14, 23, -23, 25, -25};
  int a_sq, i;

  /* white attacker: */
  if (color%2) {
    /* rook-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + rook_o[i];
      /* the king can attack from one square away: */
      if (board[a_sq] == wking) return TRUE;
      /* otherwise, check for sliding pieces: */
      while (board[a_sq] != frame) {
	if (board[a_sq] == wrook || board[a_sq] == wqueen) return TRUE;
	if (board[a_sq] != npiece) break;
	a_sq += rook_o [i];
      }
    }

    /* bishop-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + bishop_o[i];
      /* check for pawn attacks: */
      if (board[a_sq] == wpawn && i%2) return TRUE;
      /* the king can attack from one square away: */
      if (board[a_sq] == wking) return TRUE;
      while (board[a_sq] != frame) {
	if (board[a_sq] == wbishop || board[a_sq] == wqueen) return TRUE;
	if (board[a_sq] != npiece) break;
	a_sq += bishop_o [i];
      }
    }

    /* knight-style moves: */
    for (i = 0; i < 8; i++) {
      a_sq = square + knight_o[i];
      if (board[a_sq] == wknight) return TRUE;
    }

    /* if we haven't hit a white attacker by now, there are none: */
    return FALSE;

  }

  /* black attacker: */
  else {
    /* rook-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + rook_o[i];
      /* the king can attack from one square away: */
      if (board[a_sq] == bking) return TRUE;
      /* otherwise, check for sliding pieces: */
      while (board[a_sq] != frame) {
	if (board[a_sq] == brook || board[a_sq] == bqueen) return TRUE;
	if (board[a_sq] != npiece) break;
	a_sq += rook_o [i];
      }
    }

    /* bishop-style moves: */
    for (i = 0; i < 4; i++) {
      a_sq = square + bishop_o[i];
      /* check for pawn attacks: */
      if (board[a_sq] == bpawn && !(i%2)) return TRUE;
      /* the king can attack from one square away: */
      if (board[a_sq] == bking) return TRUE;
      while (board[a_sq] != frame) {
	if (board[a_sq] == bbishop || board[a_sq] == bqueen) return TRUE;
	if (board[a_sq] != npiece) break;
	a_sq += bishop_o [i];
      }
    }

    /* knight-style moves: */
    for (i = 0; i < 8; i++) {
      a_sq = square + knight_o[i];
      if (board[a_sq] == bknight) return TRUE;
    }

    /* if we haven't hit a black attacker by now, there are none: */
    return FALSE;

  }

}


void make (move_s moves[], int i) {

  /* make a move */

  /* rather than writing out from[i].from, from[i].target, etc. all over
     the place, just make a copy of them here: */
  int ep, from, target, captured, promoted, castled, find_slot;
  ep = moves[i].ep;
  from = moves[i].from;
  target = moves[i].target;
  captured = moves[i].captured;
  promoted = moves[i].promoted;
  castled = moves[i].castled;

  /* clear the en passant rights: */
  ep_square = 0;

  if (from == 0)   
    { /* drop move */
      /* Drop moves are handled fully seperate because we exepect to encouter
	 lots of them and we try to skip as many checks as possible.
	 Note that the critical path for drop moves is very short.
	 Also, we have to handle pieces[] and squares[] specially   */
      
       /* new piece on board */
      piece_count++;

      /* find first empty slot in pieces[] */
      for(find_slot = 1; (pieces[find_slot] != 0); find_slot++)
	assert(find_slot < 63);
      
      /* add to piece array, set piece-square pointer */
      pieces[find_slot] = target;

      moves[i].was_promoted = is_promoted[find_slot];
      is_promoted[find_slot] = 0;
      
      /* set square->piece pointer */
      squares[target] = find_slot;
	
      /*      if (promoted <= frame || promoted >= npiece)
      	DIE;*/

      assert(promoted > frame && promoted < npiece);
     
      DropremoveHolding(promoted, ToMove);

      /* piece went off holding but onto board */
      AddMaterial(promoted);

      /* put our piece on the board */
      board[target] = promoted;
      
      Hash(promoted,target);

      white_to_move ^= 1;

      return;
    }
  else
    {

      moves[i].was_promoted = is_promoted[squares[target]];

      /* update the "general" pieces[] / squares[] info (special moves need
	 special handling later): */
      moves[i].cap_num = squares[target];
      pieces[squares[target]] = 0;
      pieces[squares[from]] = target;
      squares[target] = squares[from];
      squares[from] = 0;

      /* update the piece count & add Holdings */
      if (!ep)
      {
	switch (board[target]) {
	case (npiece): break;
	default:
	  
	  if (Variant != Normal)
	    {
	      if (moves[i].was_promoted)
	    	{
		  addHolding(SwitchPromoted(board[target]), ToMove);
	    	}
	      else
	    	{ 
		  addHolding(SwitchColor(board[target]), ToMove);
	    	}
	    }
	  
	  RemoveMaterial(board[target]);
	  
	  /* remove captured piece */
	  Hash(board[target], target);
	  
	  piece_count--;
	  break;
	}
      }

      /* white pawn moves: */
      if (board[from] == wpawn) {
	/* look for a promotion move: */
	if (promoted) {
	  board[target] = promoted;
	  board[from] = npiece;
	  moved[target]++;
	  moved[from]++;
	  white_to_move ^= 1;

	  is_promoted[squares[target]] = 1;

	  /* remove pawn */
	  Hash(wpawn, from);
	  /* add new stuff */
	  Hash(promoted, target);

	  RemoveMaterial(wpawn);
	  AddMaterial(promoted);

	  return;
	}

	/* look for an en passant move: */
	if (ep) {
	  
	  /* remove pawn */
	  Hash(wpawn, from);
	  /* remove ep pawn */
	  Hash(bpawn, target-12);
	  /* add target pawn */
	  Hash(wpawn, target);

	  RemoveMaterial(bpawn);

	  board[target] = wpawn;
	  board[from] = npiece;

	  addHolding(wpawn, WHITE);
	  piece_count--;

	  board[target-12] = npiece;
	  moved[target]++;
	  moved[from]++;
	  moved[target-12]++;
	  white_to_move ^= 1;
	  moves[i].cap_num = squares[target-12];

	  pieces[squares[target-12]] = 0;
	  squares[target-12] = 0;

	  return;
	}
	
	/* otherwise, we have a "regular" pawn move: */
	/* first check to see if we've moved a pawn up 2 squares: */
	if (target == from+24)
	  ep_square = from+12;

	Hash(wpawn, from);
	Hash(wpawn, target);
	
	board[target] = wpawn;
	board[from] = npiece;
	moved[target]++;
	moved[from]++;
	white_to_move ^= 1;
	return;
	
      }
      
      /* black pawn moves: */
      if (board[from] == bpawn) {
	/* look for a promotion move: */
	if (promoted) {
	  board[target] = promoted;
	  board[from] = npiece;
	  moved[target]++;
	  moved[from]++;
	  white_to_move ^= 1;

	  is_promoted[squares[target]] = 1;

	  /* remove pawn */
	  Hash(bpawn, from);
	  /* add new stuff */
	  Hash(promoted, target);

	  RemoveMaterial(bpawn);
	  AddMaterial(promoted);

	  return;
	}
	
	/* look for an en passant move: */
	if (ep) {

	  /* remove pawn */
	  Hash(bpawn, from);
	  /* remove ep pawn */
	  Hash(wpawn, target+12);
	  /* add target pawn */
	  Hash(bpawn, target);

	  RemoveMaterial(wpawn);

	  board[target] = bpawn;
	  board[from] = npiece;

	  addHolding(bpawn, BLACK);
	  piece_count--;

	  board[target+12] = npiece;
	  moved[target]++;
	  moved[from]++;
	  moved[target+12]++;
	  white_to_move ^= 1;
	  moves[i].cap_num = squares[target+12];
	  pieces[squares[target+12]] = 0;
	  squares[target+12] = 0;
	  return;
	}
	
	/* otherwise, we have a "regular" pawn move: */
	/* first check to see if we've moved a pawn down 2 squares: */
	if (target == from-24)
	  ep_square = from-12;
	
	board[target] = bpawn;
	board[from] = npiece;
	moved[target]++;
	moved[from]++;
	white_to_move ^= 1;

	Hash(bpawn, from);
	Hash(bpawn, target);

	return;
      }
      
      /* piece moves, other than the king: */
      if (board[from] != wking && board[from] != bking) {
	
	Hash(board[from], from);
	Hash(board[from], target);

	board[target] = board[from];
	board[from] = npiece;
	moved[target]++;
	moved[from]++;
	white_to_move ^= 1;
	return;
      }

      /* otherwise, we have a king move of some kind: */
      /* White king moves first: */
      if (board[from] == wking) {
	/* record the new white king location: */
	wking_loc = target;
	
	/* perform the white king's move: */
	board[target] = wking;
	board[from] = npiece;
	moved[target]++;
	moved[from]++;
	white_to_move ^= 1;

	Hash(wking, from);
	Hash(wking, target);
	
	/* check for castling: */
	/* check for white kingside castling: */
	if (castled == wck) {
	  board[33] = npiece;
	  board[31] = wrook;
	  moved[33]++;
	  moved[31]++;
	  white_castled = wck;
	  pieces[squares[33]] = 31;
	  squares[31] = squares[33];
	  squares[33] = 0;
	  
	  Hash(wrook, 33);
	  Hash(wrook, 31);

	  return;
	}
	
	/* check for white queenside castling: */
	else if (castled == wcq) {
	  board[26] = npiece;
	  board[29] = wrook;
	  moved[26]++;
	  moved[29]++;
	  white_castled = wcq;
	  pieces[squares[26]] = 29;
	  squares[29] = squares[26];
	  squares[26] = 0;

	  Hash(wrook, 26);
	  Hash(wrook, 29);
	  
	  return;
	}
	
	return;
      }
      
      /* now we have only black king moves left: */
      else {
	/* record the new black king location: */
	bking_loc = target;
	
	/* perform the black king's move: */
	board[target] = bking;
	board[from] = npiece;
	moved[target]++;
	moved[from]++;
	white_to_move ^= 1;

	Hash(bking, from);
	Hash(bking, target);
	
	/* check for castling: */
	/* check for black kingside castling: */
	if (castled == bck) {
	  board[117] = npiece;
	  board[115] = brook;
	  moved[117]++;
	  moved[115]++;
	  black_castled = bck;
	  pieces[squares[117]] = 115;
	  squares[115] = squares[117];
	  squares[117] = 0;

	  Hash(brook, 117);
	  Hash(brook, 115);

	  return;
	}

	/* check for black queenside castling: */
	else if (castled == bcq) {
	  board[110] = npiece;
	  board[113] = brook;
	  moved[110]++;
	  moved[113]++;
	  black_castled = bcq;
	  pieces[squares[110]] = 113;
	  squares[113] = squares[110];
	  squares[110] = 0;

	  Hash(brook, 110);
	  Hash(brook, 113);

	  return;
	}
      }
    return;
  }
}

void try_drop (move_s moves[], int *num_moves, int ptype, int target)
{
   
  moves[*num_moves].from = 0;
  moves[*num_moves].target = target;
  moves[*num_moves].captured = board[target]; /* should be npiece */
  moves[*num_moves].castled = no_castle;
  moves[*num_moves].promoted = ptype;
  moves[*num_moves].ep = FALSE;
  moves[*num_moves].was_promoted = FALSE;
  (*num_moves)++;

  return;
  
}

void push_king (move_s moves[], int *num_moves, int from, int target, 
		int castle_type) {

  /* add king moves to the moves array */

  /* first see if the move will take the king off the board: */
  if (board[target] == frame)
    return;

  /* check to see if we have a non capture when in qsearch: */
  if (board[target] == npiece && captures)
    return;

  /* check for a castling move: */
  if (castle_type) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = npiece;
    moves[*num_moves].castled = castle_type;
    moves[*num_moves].promoted = 0;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;
    return;
  }

  /* non-capture, 'normal' king moves: */
  if (board[target] == npiece) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = npiece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = 0;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;
    return;
  }

  /* 'normal' caputure moves by the king: */
  else if (board[target]%2 != board[from]%2) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = board[target];
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = 0;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;
    return;
  }

  /* no more possible moves for the king, so return: */
  return;
}


void push_knight (move_s moves[], int *num_moves, int from, int target) {

  /* add knight moves to the moves array */

  /* test if the move will take the knight off the board: */
  if (board[target] == frame)
    return;

  /* check to see if we have a non capture when in qsearch: */
  if (board[target] == npiece && captures)
    return;

  /* check for a non-capture knight move: */
  if (board[target] == npiece) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = npiece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = 0;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;
    return;
  }

  /* check for a capture knight move: */
  else if (board[target]%2 != board[from]%2) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = board[target];
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = 0;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;
    return;
  }

  /* no more possible moves left for the knight, so return: */
  return;
}


void push_pawn (move_s moves[], int *num_moves, int from, int target, 
		bool is_ep) {

  /* add pawn moves to the moves array */

  int captured_piece;

  /* first check to see if we're going to go off the board first: */
  if (board[target] == frame)
    return;

  /* check to see if it's an ep move: */
  if (is_ep) {
    if (board[from] == wpawn) {
      moves[*num_moves].from = from;
      moves[*num_moves].target = target;
      moves[*num_moves].captured = bpawn;
      moves[*num_moves].castled = no_castle;
      moves[*num_moves].promoted = 0;
      moves[*num_moves].ep = TRUE;
      moves[*num_moves].was_promoted = FALSE;
      (*num_moves)++;
      return;
    }
    else {
      moves[*num_moves].from = from;
      moves[*num_moves].target = target;
      moves[*num_moves].captured = wpawn;
      moves[*num_moves].castled = no_castle;
      moves[*num_moves].promoted = 0;
      moves[*num_moves].ep = TRUE;
      moves[*num_moves].was_promoted = FALSE;
      (*num_moves)++;
      return;
    }
  }

  /* record which piece we are taking, so we don't have to compute it over
     and over again: */
  captured_piece = board[target];
  
  /* look for a white promotion move: */
  if (board[from] == wpawn && rank(from) == 7) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = wqueen;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = wrook;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = wknight;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = wbishop;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    /* we've finished generating all the promotions: */
    return;
  }

  /* look for a black promotion move: */
  else if (board[from] == bpawn && rank(from) == 2) {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = bqueen;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = brook;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = bknight;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = bbishop;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;

    /* we've finished generating all the promotions: */
    return;
  }

  /* otherwise, we have a normal pawn move: */
  else {
    moves[*num_moves].from = from;
    moves[*num_moves].target = target;
    moves[*num_moves].captured = captured_piece;
    moves[*num_moves].castled = no_castle;
    moves[*num_moves].promoted = 0;
    moves[*num_moves].ep = FALSE;
    moves[*num_moves].was_promoted = FALSE;
    (*num_moves)++;
    return;
  }

  /* the function should never get here, but just for completeness: */
  return;
}


void push_slide (move_s moves[], int *num_moves, int from, int target) {

  /* add moves for sliding pieces to the moves array */

  int offset;
  bool hit_piece;

  /* check to see if we have gone off the board first: */
  if (board[target] == frame)
    return;

  /* init variables: */
  offset = target - from;
  hit_piece = FALSE;

  /* loop until we hit the edge of the board, or another piece: */
  do {
    /* case when the target is an empty square: */
    if (board[target] == npiece) {
      if (!captures) {
	moves[*num_moves].from = from;
	moves[*num_moves].target = target;
	moves[*num_moves].captured = npiece;
	moves[*num_moves].castled = no_castle;
	moves[*num_moves].promoted = 0;
	moves[*num_moves].ep = FALSE;
	moves[*num_moves].was_promoted = FALSE;
	(*num_moves)++;
      }
      target += offset;
    }

    /* case when an enemy piece is hit: */
    else if (board[target]%2 != board[from]%2) {
      moves[*num_moves].from = from;
      moves[*num_moves].target = target;
      moves[*num_moves].captured = board[target];
      moves[*num_moves].castled = no_castle;
      moves[*num_moves].promoted = 0;
      moves[*num_moves].ep = FALSE;
      moves[*num_moves].was_promoted = FALSE;
      (*num_moves)++;
      hit_piece = TRUE;
    }

    /* otherwise, we have hit a friendly piece (or edge of board): */
    else
      hit_piece = TRUE;
  } while (!hit_piece && board[target] != frame);

  /* we have finished generating all of the sliding moves, so return: */
  return;

}


void unmake (move_s moves[], int i) {

  /* un-make a move */

  /* rather than writing out from[i].from, from[i].target, etc. all over
     the place, just make a copy of them here: */
  int ep, from, target, captured, promoted, castled;
  ep = moves[i].ep;
  from = moves[i].from;
  target = moves[i].target;
  captured = moves[i].captured;
  promoted = moves[i].promoted;
  castled = moves[i].castled;

  if (from == 0)   /* drop move */
    {
      /* Drop moves are hanled fully seperate because we exepect to encouter
	 lots of them and we try to skip as many checks as possible.
	 Note that the critical path for drop moves is very short.
	 Also, we have to handle pieces[] and squares[] specially   */
      
       /* remove from piece array, unset piece-square pointer */

       pieces[squares[target]] = 0;
       is_promoted[squares[target]] = moves[i].was_promoted;
       
       /* unset square->piece pointer */
       squares[target] = 0;

       piece_count--;

       assert(promoted < npiece && promoted > frame);
       
       DropaddHolding(promoted, NotToMove);

       RemoveMaterial(promoted);

       /* restore board, either no piece or ep square */
       board[target] = captured;

       Hash(promoted,target);

       white_to_move ^= 1;

       return;
    }
  else
    {
      
      /* update the "general" pieces[] / squares[] info (special moves need
	 special handling later): */
      
      squares[from] = squares[target];
      squares[target] = moves[i].cap_num;
      pieces[squares[target]] = target;
      pieces[squares[from]] = from;
      
      is_promoted[squares[target]] = moves[i].was_promoted;
      
      /* update the piece count for determining opening/middlegame/endgame stage */
      if (!ep)
	{
	  switch (captured) {
	  case (npiece): break;
	  default:
	    
	    if (Variant != Normal)
	      {
		if (is_promoted[squares[target]])
		  {
		    removeHolding(SwitchPromoted(captured), NotToMove); 
		  }
		else
		  { 
		removeHolding(SwitchColor(captured), NotToMove);
		  } 
	      }
	
	    Hash(captured, target);
	    
	    AddMaterial(captured);
	    
	    piece_count++; 
	    break;
	  }
	}
      
      /* white pawn moves: */
      if (board[target] == wpawn) {
	/* look for an en passant move: */
	if (ep) {
	  
	  Hash(wpawn, target);
	  Hash(wpawn, from);
	  Hash(bpawn, target-12);

	  board[target] = npiece;
	  board[from] = wpawn;

	  AddMaterial(bpawn);

	  removeHolding(wpawn, WHITE);
	  piece_count++;

	  board[target-12] = bpawn;
	  moved[target]--;
	  moved[from]--;
	  moved[target-12]--;
	  white_to_move ^= 1;
	  squares[target-12] = moves[i].cap_num;
	  pieces[moves[i].cap_num] = target-12;
	  squares[target] = 0;
	  return;
	}
	
	/* otherwise, we have a "regular" pawn move: */
	Hash(wpawn, from);
	Hash(wpawn, target);

	board[target] = captured;
	board[from] = wpawn;
	moved[target]--;
	moved[from]--;
	white_to_move ^= 1;
	return;
	
      }
      
      /* black pawn moves: */
      if (board[target] == bpawn) {
	/* look for an en passant move: */
	if (ep) {

	  Hash(bpawn, target);
	  Hash(bpawn, from);
	  Hash(wpawn, target+12);

	  board[target] = npiece;
	  board[from] = bpawn;

	  AddMaterial(wpawn);

	  removeHolding(bpawn, BLACK);
	  piece_count++;

	  board[target+12] = wpawn;
	  moved[target]--;
	  moved[from]--;
	  moved[target+12]--;
	  white_to_move ^= 1;
	  squares[target+12] = moves[i].cap_num;
	  pieces[moves[i].cap_num] = target+12;
	  squares[target] = 0;
	  return;
	}
	
	Hash(bpawn, from);
	Hash(bpawn, target);

	/* otherwise, we have a "regular" pawn move: */
	board[target] = captured;
	board[from] = bpawn;
	moved[target]--;
	moved[from]--;
	white_to_move ^= 1;
	return;
	
      }
      
      /* piece moves, other than the king: */
      if (board[target] != wking && board[target] != bking && !promoted) {
	board[from] = board[target];
	board[target] = captured;
	moved[target]--;
	moved[from]--;
	white_to_move ^= 1;
	
	Hash(board[from], target);
	Hash(board[from], from);

	return;
      }
      
      /* look for a promotion move: */
      if (promoted) {
	/* white promotions: */
	if (board[target]%2) {
	  board[target] = captured;
	  board[from] = wpawn;
	  moved[target]--;
	  moved[from]--;
	  white_to_move ^= 1;

	  Hash(wpawn, from);
	  Hash(promoted, target);

	  RemoveMaterial(promoted);
	  AddMaterial(wpawn);

	  return;
	}
	
	/* black promotions: */
	board[target] = captured;
	board[from] = bpawn;
	moved[target]--;
	moved[from]--;
	white_to_move ^= 1;

	Hash(bpawn, from);
	Hash(promoted, target);

	RemoveMaterial(promoted);
	AddMaterial(bpawn);

	return;
      }
      
      /* otherwise, we have a king move of some kind: */
      /* White king moves first: */
      if (board[target] == wking) {
	/* record the new white king location: */
	wking_loc = from;

	/* perform the white king's move: */
	board[target] = captured;
	board[from] = wking;
	moved[target]--;
	moved[from]--;
	white_to_move ^= 1;

	Hash(wking, from);
	Hash(wking, target);
	
	/* check for castling: */
	/* check for white kingside castling: */
	if (castled == wck) {
	  board[33] = wrook;
	  board[31] = npiece;
	  moved[33]--;
	  moved[31]--;
	  white_castled = no_castle;
	  squares[33] = squares[31];
	  squares[31] = 0;
	  pieces[squares[33]] = 33;

	  Hash(wrook, 33);
	  Hash(wrook, 31);

	  return;
	}
	
	/* check for white queenside castling: */
	else if (castled == wcq) {
	  board[26] = wrook;
	  board[29] = npiece;
	  moved[26]--;
	  moved[29]--;
	  white_castled = no_castle;
	  squares[26] = squares[29];
	  squares[29] = 0;
	  pieces[squares[26]] = 26;

	  Hash(wrook, 29);
	  Hash(wrook, 26);

	  return;
	}
	
	return;
      }
      
      /* now we have only black king moves left: */
      else {
	/* record the new black king location: */
	bking_loc = from;
	
	/* perform the black king's move: */
	board[target] = captured;
	board[from] = bking;
	moved[target]--;
	moved[from]--;
	white_to_move ^= 1;

	Hash(bking, from);
	Hash(bking, target);
	
	/* check for castling: */
	/* check for black kingside castling: */
	if (castled == bck) {
	  board[117] = brook;
	  board[115] = npiece;
	  moved[117]--;
	  moved[115]--;
	  black_castled = no_castle;
	  squares[117] = squares[115];
	  squares[115] = 0;
	  pieces[squares[117]] = 117;

	  Hash(brook, 117);
	  Hash(brook, 115);

	  return;
	}
	
	/* check for black queenside castling: */
	else if (castled == bcq) {
	  board[110] = brook;
	  board[113] = npiece;
	  moved[110]--;
	  moved[113]--;
	  black_castled = no_castle;
	  squares[110] = squares[113];
	  squares[113] = 0;
	  pieces[squares[110]] = 110;

	  Hash(brook, 110);
	  Hash(brook, 113);

	  return;
	}
      }
    }
  return;
}
