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

    File: search.c                                        
    Purpose: contains functions related to the recursive search

*/

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"
#include "limits.h"

unsigned long FH, FHF;
unsigned long razor_drop, razor_material;

unsigned long rep_list[64];

char true_i_depth;

int bestmovenum;

void order_moves (move_s moves[], long int move_ordering[], int num_moves, int best) {

  /* sort out move ordering scores in move_ordering, using implemented
     heuristics: */

  int cap_values[14] = {
    0,100,100,210,210,5000,5000,250,250,450,450,230,230,0};

  int scap_values[14] = {
    0,100,100,310,310,5000,5000,500,500,900,900,325,325,0};
    
  int i, from, target, promoted, captured;

  /* fill the move ordering array: */

  /* if searching the pv, give it the highest move ordering, and if not, rely
     on the other heuristics: */
  if (searching_pv) {
    searching_pv = FALSE;
    for (i = 0; i < num_moves; i++) {
      from = moves[i].from;
      target = moves[i].target;
      promoted = moves[i].promoted;
      captured = moves[i].captured;
      
      /* give captures precedence in move ordering, and order captures by
	 material gain */
      if (captured != npiece)
	{
	  if (Variant == Normal)
	    move_ordering[i] = scap_values[captured]-cap_values[board[from]]+50000;    
	  else
	    move_ordering[i] = cap_values[captured]-cap_values[board[from]]+50000;
	}      
      else
	move_ordering[i] = 0;
      
      /* make the pv have highest move ordering: */
      if (from == pv[1][ply].from 
	  && target == pv[1][ply].target
	  && promoted == pv[1][ply].promoted) {
	searching_pv = TRUE;
	move_ordering[i] += INF;
      } 
      else if ((best != -1) && (best != -2) && (i == best))
	{
	  move_ordering[i] += INF;
	}
      else if (best == -2)
	{
	  /* we have an iterative deepening move */
	  if (from == pv[ply+1][ply+1].from 
	      && target == pv[ply+1][ply+1].target 
	      && promoted == pv[ply+1][ply+1].promoted)
	    {
	      move_ordering[i] += INF;
	    }
	}
      
      /* heuristics other than pv (no need to use them on the pv move - it is
	 already ordered highest) */
      else {
	/* add the history heuristic bonus: */
	move_ordering[i] += (history_h[from][target]>>i_depth);

	/* add the killer move heuristic bonuses: */
	if (from == killer1[ply].from && target == killer1[ply].target
	    && promoted == killer1[ply].promoted)
	  move_ordering[i] += 10000;
	else if (from == killer2[ply].from && target == killer2[ply].target
	    && promoted == killer2[ply].promoted)
	  move_ordering[i] += 5000;
	else if (from == killer3[ply].from && target == killer3[ply].target
	    && promoted == killer3[ply].promoted)
	  move_ordering[i] += 2500;
	/* idea from Ed Schroeder: use prev and next ply killers */
	else if ((ply > 1) 
		 && from == killer1[ply-1].from 
		 && target == killer1[ply-1].target
		 && promoted == killer1[ply-1].promoted)
	  move_ordering[i] += 2000;
	else if (from == killer1[ply+1].from 
		 && target == killer1[ply+1].target
		 && promoted == killer1[ply+1].promoted)
	  move_ordering[i] += 1500;
      }
    }
  }

  /* if not searching the pv: */
  else {
    for (i = 0; i < num_moves; i++) {
      from = moves[i].from;
      target = moves[i].target;
      promoted = moves[i].promoted;
      captured = moves[i].captured;
      
      /* give captures precedence in move ordering, and order captures by
	 material gain */
      if ((best != -1) && (i == best))
	{
	  move_ordering[i] += INF;
	}
      else if (best == -2)
	{
	  /* we have an iterative deepening move */
	  if (from == pv[ply+1][ply+1].from 
	      && target == pv[ply+1][ply+1].target 
	      && promoted == pv[ply+1][ply+1].promoted)
	    {
	      move_ordering[i] += INF;
	    }
	}
      else if (captured != npiece)
	{	  
	  if (Variant == Normal)
	    move_ordering[i] = scap_values[captured]-cap_values[board[from]]+50000;    
	  else
	    move_ordering[i] = cap_values[captured]-cap_values[board[from]]+50000;
	}      
      else
	move_ordering[i] = 0;
      
      /* heuristics other than pv */
      
      /* add the history heuristic bonus: */
      move_ordering[i] += (history_h[from][target]>>i_depth);

      /* add the killer move heuristic bonuses: */
      if (from == killer1[ply].from && target == killer1[ply].target
	  && promoted == killer1[ply].promoted)
	move_ordering[i] += 10000;
      else if (from == killer2[ply].from && target == killer2[ply].target
	  && promoted == killer2[ply].promoted)
	move_ordering[i] += 5000;
      else if (from == killer3[ply].from && target == killer3[ply].target
	  && promoted == killer3[ply].promoted)
	move_ordering[i] += 2500;
	/* idea from Ed Schroeder: use prev and next ply killers */
      else if ((ply > 1) 
	       && from == killer1[ply-1].from 
	       && target == killer1[ply-1].target
	       && promoted == killer1[ply-1].promoted)
	move_ordering[i] += 2000;
      else if (from == killer1[ply+1].from 
	       && target == killer1[ply+1].target
	       && promoted == killer1[ply+1].promoted)
	move_ordering[i] += 1500;
    }
  }

}

void perft (int depth) {

  move_s moves[MOVE_BUFF];
  int num_moves, i, ep_temp;

  ep_temp = ep_square;
  num_moves = 0;

  /* return if we are at the maximum depth: */
  if (!depth) {
    return;
  }

  /* generate the move list: */
  gen (&moves[0], &num_moves);

  /* loop through the moves at the current depth: */
  for (i = 0; i < num_moves; i++) {
    make (&moves[0], i);

    /* check to see if our move is legal: */
    if (check_legal (&moves[0], i)) {
      raw_nodes++;
      /* go deeper into the tree recursively, increasing the indent to
	 create the "tree" effect: */
      perft (depth-1);
    }

    /* unmake the move to go onto the next: */
    unmake (&moves[0], i);
  }

  ep_square = ep_temp;

}


long int qsearch (int alpha, int beta, int depth) {

  /* perform a quiscense search on the current node using alpha-beta with
     negamax search */

  move_s moves[MOVE_BUFF];
  int num_moves, i, j;
  long int score = -INF, standpat, move_ordering[MOVE_BUFF];
  bool legal_move, no_moves = TRUE;
  int sbest, best_score, best, delta;
  int lzscore;
  
  /* return our score if we're at a leaf node: */
  if (depth <= 0) {
    /* remove leafcounting effect */
    qnodes--;
    score = eval ();
    return score;
  }

  /* before we do anything, see if we're out of time: */
  if (!(nodes & 4095)) {
    if ((interrupt() && !go_fast) || (rdifftime (rtime (), start_time) >= time_for_move)) {
      time_exit = TRUE;
      return 0;
    }
  }
  
  best = -1;

#ifdef LAZYEVAL
  lzscore = white_to_move ? Material : -Material;

  if (lzscore - 250 >= beta)
  {
  	return beta;
  }
  else if (lzscore + 250 < alpha)
  {
  	/*standpat = alpha;*/standpat = lzscore;
  }
  else
  {
  	standpat = eval ();
  }
#else
  standpat = eval();
#endif

  if (standpat >= beta) {
    return standpat;        /* standpat */
  }
  else if (standpat > alpha) {
    alpha = standpat;
  }

  pv_length[ply] = ply;

  num_moves = 0;
  sbest = -1;
  best_score = -INF;
    
  delta = alpha-(Variant == Normal ? 100 : 200)-standpat;

  /* generate and order moves: */
  gen (&moves[0], &num_moves);
  order_moves (&moves[0], &move_ordering[0], num_moves, best);

  /* loop through the moves at the current node: */
  while (remove_one (&i, &move_ordering[0], num_moves)) {

    make (&moves[0], i);
    
    ply++;
    legal_move = FALSE;

    /* go deeper if it's a legal move: */
    if (check_legal (&moves[0], i)) {
      /* check whether it is a futile capture : 
	 captured piece wont bring us back near alpha */
      /* warning : promotions should be accounted for */
      /* because the piece will go to our holdings, its
	 value is essentially double */
      DeltaTries++;

      if ((abs(material[moves[i].captured] * (Variant == Normal?1:2)) >= delta) || (moves[i].promoted))
        {
	  nodes++;
	  qnodes++;
	  score = -qsearch (-beta, -alpha, depth-1);
	  legal_move = TRUE;
	  no_moves = FALSE;
	}
      else DeltaCuts++;
    }

    unmake (&moves[0], i);
    ply--;

    if(score > best_score) best_score = score;

    /* check our current score vs. alpha: */
    if (score > alpha && legal_move) {

      /* don't update the history heuristic scores here, since depth is messed
	 up when qsearch is called */
      best = i;

      /* try for an early cutoff: */
      if (score >= beta) {
	return score;
      }
      
      alpha = score;
      
      /* update the pv: */
      pv[ply][ply] = moves[i];
      for (j = ply+1; j < pv_length[ply+1]; j++)
	pv[ply][j] = pv[ply+1][j];
      pv_length[ply] = pv_length[ply+1];
    }

  }

  /* we don't check for mate / stalemate here, because without generating all
     of the moves leading up to it, we don't know if the position could have
     been avoided by one side or not */

  if (no_moves)
    return alpha;
  else
    return best_score;
  
}


bool remove_one (int *marker, long int move_ordering[], int num_moves) {

  /* a function to give pick the top move order, one at a time on each call.
     Will return TRUE while there are still moves left, FALSE after all moves
     have been used */

  int i, best = -1;

  *marker = -1;

  for (i = 0; i < num_moves; i++) {
    if (move_ordering[i] > best) {
      *marker = i;
      best = move_ordering[i];
    }
  }

  if (*marker > -1) {
    move_ordering[*marker] = -1;
    return TRUE;
  }
  else {
    return FALSE;
  }

}


long int search (int alpha, int beta, int depth, bool is_null) {

  /* search the current node using alpha-beta with negamax search */

  move_s moves[MOVE_BUFF];
  int num_moves, i, j;
  long int score = -INF, move_ordering[MOVE_BUFF];
  bool no_moves, legal_move;
  int bound, threat = 0, donull, best, sbest, best_score, old_ep;
  bool incheck, first;
  int extend = 0, fscore, fmax, selective = 0;
  int rep_loop;

  /* before we do anything, see if we're out of time: */
  if (!(nodes & 4095)) {
    if ((interrupt() && !go_fast) || (rdifftime (rtime (), start_time) >= time_for_move)) {
      time_exit = TRUE;
      return 0;
    }
  }

  /* maybe optimize this so we step 2 at time ? */
  for (rep_loop = 1;rep_loop < ply; rep_loop++)
    {
      if (rep_list[rep_loop] == hash)
	return 0;  /* draw-by-rep */
    }
  
  rep_list[ply] = hash;
  
  incheck = in_check();

  /* perform check extensions if we haven't gone past maxdepth: */
  if (ply < maxdepth+1 && incheck && ((depth <= 0) || (ply <= (i_depth*2)))) 
    {
      depth++;
      ext_check++;
      extend++;
    }
  
  /* try to find a stable position before passing the position to eval (): */
  if (depth <= 0) {
    
    captures = TRUE;
    score = qsearch (alpha, beta, ply * 2);   
    captures = FALSE;

    return score; /*score;*/
  }

  num_moves = 0;
  no_moves = TRUE;

  switch (ProbeTT(&bound, alpha, beta, &best, &threat, &donull, depth))
    {
    case EXACT:
      return bound;
      break;
    case UPPER:
      if (bound <= alpha)
	return bound;
      if (bound < beta)
      	beta = bound;
      break;
    case LOWER:
      if (bound >= beta)
	return bound;
      if (bound > alpha)
      	alpha = bound;
      break;
    case DUMMY:
      break;
    case HMISS:
      best = -1;
      threat = FALSE;
      break;
     };
   
  sbest = -1;
  best_score = -INF;

  old_ep = ep_square;

  if (((piece_count > 6) || (depth < 5)) && (is_null == FALSE) && !incheck && donull && (threat == FALSE))
    {

      ep_square = 0;      
      white_to_move ^= 1;
      ply++;
      hash ^= 0xDEADBEEF;

	/* use R=1 cos R=2 is too dangerous for our ply depths */
      if (Variant != Normal)
	score = -search(-beta, -beta+1, ((depth > 3) ? depth-2-1 : depth-1-1), TRUE);
      else
	score = -search(-beta, -beta+1, ((depth > 6) ? depth-3-1 : depth-2-1), TRUE);	
      white_to_move ^= 1;
      ply--;
      hash ^= 0xDEADBEEF;
      ep_square = old_ep;

      if (time_exit) return 0;

      NTries++;

      if (depth > 3) NDTries++;

      if (score >= beta)
	{
	  if (depth > 3) NDCuts++;
	  
	  NCuts++;
	  
	  StoreTT(score, alpha, beta, 0, 0, depth);
	  
	  return score;
	}
      else if (score < -INF+100)
	{
	  threat = TRUE;
	  TExt++;
	  depth++;
	  extend++;
	}
    }
  else if (threat == TRUE)
    {
      TExt++;
      depth++;
      extend++;
    }

  pv_length[ply] = ply;
 
#define IDEEP
#ifdef IDEEP
  /* Internal Iterative deepening */
  if ((best == -1) && (depth > 2) && (searching_pv)) 
    {
     score = search (alpha, beta, depth-2, TRUE);
     ep_square = old_ep;
      if (time_exit) return 0;
      if (score <= alpha)
	{
	  score = search ( -INF, beta, depth-2, TRUE);
	  ep_square = old_ep;	  
   	if (time_exit) return 0;
	}
      best = -2;
    }
#endif

  score = -INF;
  
  first = TRUE;

  fscore = (white_to_move ? Material : -Material) + (Variant == Normal ? 900 : 450);

  if (!extend && depth == 3 && fscore <= alpha)
    depth = 2;
  
  fscore = (white_to_move ? Material : -Material) + (Variant == Normal ? 500 : 250);

  if (!extend && depth == 2 && fscore <= alpha)
    {
      selective = 1;
      best_score = fmax = fscore;
    }

  fscore = (white_to_move ? Material : -Material) + (Variant == Normal ? 100 : 200);

  if (!extend && depth == 1 && fscore <= alpha)
    {
      selective = 1;
      best_score = fmax = fscore;
    }

  /* generate and order moves: */
  gen (&moves[0], &num_moves);
  order_moves (&moves[0], &move_ordering[0], num_moves, best);

  /* loop through the moves at the current node: */
  while (remove_one (&i, &move_ordering[0], num_moves)) {

    make (&moves[0], i);
    ply++;
    legal_move = FALSE;

     old_ep = ep_square;
	
    /* go deeper if it's a legal move: */
     
     if (check_legal (&moves[0], i)) {

      extend = 0;

      /* Razoring of uninteresting drops */
        if ((moves[i].from == 0)
	&& (depth > 1)   /* more than pre-frontier nodes */
        && !in_check()   /* not a checking move */
	&& !incheck      /* not a check evasion */
	&& !first        /* dont want to razor the PV! */
	)
	{ razor_drop++; extend--;};

      if (!selective 
	  || in_check() 
	  || (fmax + (abs(material[moves[i].captured]) * (Variant == Normal?1:2)) > alpha) 
	  || (moves[i].promoted)) 
      {
	
        /* we only count the nodes we actually examine */  

        nodes++;

	/* Dynamically decide whether it would be safe to do PVS */
	
	if ((first == TRUE) || (best == -1))
	  { 
	    ep_square = old_ep;
	    score = -search (-beta, -alpha, depth+extend-1, FALSE);
	    ep_square = old_ep;
	    FULL++;
	  }
	else
	  {
	    ep_square = old_ep;
	    score = -search (-alpha-1, -alpha, depth+extend-1, FALSE);
	    PVS++;
	    ep_square = old_ep;	    

	    if (score > best_score && !time_exit)
	      {
		if ((score > alpha) && (score < beta) && !time_exit)
		  {
		    score = -search(-beta, -score, depth+extend-1, FALSE);
		    ep_square = old_ep;
		    PVSF++;
		  }
	      }
	  }
	legal_move = TRUE;
      }
      else
	razor_material++;

      no_moves = FALSE;
	
    }

    if (score > best_score) best_score = score;

    unmake (&moves[0], i);
    ply--;

    /* return if we've run out of time: */
    if (time_exit) return 0;

    /* check our current score vs. alpha: */
    if (score > alpha && legal_move) {
      
       /* try for an early cutoff: */
      if (score >= beta) {

	/* update the history heuristic since we have a cutoff: */
	history_h[moves[i].from][moves[i].target] += depth * depth;

	if (moves[i].captured == npiece)
	  {
	    /* we have a cutoff, so update our killers: */
	    if (score > killer_scores[ply]) {
	      killer_scores2[ply] = killer_scores[ply];
	      killer_scores[ply] = score;
	      killer3[ply] = killer2[ply];
	      killer2[ply] = killer1[ply];
	      killer1[ply] = moves[i];
	    }
	    else if (score > killer_scores2[ply]) {
	      killer_scores2[ply] = score;
	      killer3[ply] = killer2[ply];
	      killer2[ply] = moves[i];
	    };
	  }

	if (first == TRUE) FHF++;

	FH++;
	
	StoreTT(score, alpha, beta, i, threat, depth);

	return score;
      }
      
      alpha = score;

      sbest = i;

      /* update the pv: */
      pv[ply][ply] = moves[i];
      for (j = ply+1; j < pv_length[ply+1]; j++)
	pv[ply][j] = pv[ply+1][j];
      pv_length[ply] = pv_length[ply+1];
    }

    if (legal_move)
      first = FALSE;

  }

  /* check for mate / stalemate: */
  if (no_moves) {
    if (in_check ()) {

      StoreTT(-INF+ply, alpha, beta, 0, threat, depth);

      return (-INF+ply);
    }
    else {

      StoreTT(0, alpha, beta, 0, threat, depth);

      return 0;
    }
  }

  if (best_score <= alpha)
    {
      if (!selective)
	StoreTT(best_score, alpha, beta, sbest, threat, depth);
    }
  else 
    {
      if (!selective)
	StoreTT(best_score, alpha, beta, sbest, threat, depth);
      else
	StoreTT(best_score, -INF, -INF, sbest, threat, depth);/*store lowbound*/
    }
 
  return best_score;

}


move_s search_root (int originalalpha, int originalbeta, int depth) {

  /* search the root node using alpha-beta with negamax search */

  move_s moves[MOVE_BUFF], best_move = dummy;
  int num_moves, i, j;
  long int root_score = -INF, move_ordering[MOVE_BUFF];
  bool no_moves, legal_move, first, found_move;
  int old_ep;
  int alpha, beta;

  alpha = originalalpha;
  beta = originalbeta;

  num_moves = 0;
  no_moves = TRUE;
  ply = 1;
  searching_pv = TRUE;
  time_exit = FALSE;
  time_failure = FALSE;
  first = TRUE;
  cur_score = -INF;

  pv_length[ply] = ply;

  rep_list[ply] = hash;

  /* check extensions: */
 
  if (in_check ()) {ext_check++;depth++;};

  /* generate and order moves: */
  gen (&moves[0], &num_moves);
  order_moves (&moves[0], &move_ordering[0], num_moves, -1);

  /* loop through the moves at the root: */
  while (remove_one (&i, &move_ordering[0], num_moves)) {

    make (&moves[0], i);
    ply++;
    legal_move = FALSE;
	
    old_ep = ep_square;

    /* go deeper if it's a legal move: */
    if (check_legal (&moves[0], i)) {
      nodes++;

      if ((first == TRUE) || (i_depth < 2))
	{
	  root_score = -search (-beta, -alpha, depth-1, FALSE);
	  ep_square = old_ep;	

	  if (first && !time_exit && (post || !xb_mode) && i_depth >= mindepth) 
	    {
	      if (root_score >= beta)
		{
		  /* update the pv: */
		  pv[ply-1][ply-1] = moves[i];
		  for (j = ply; j < pv_length[ply]; j++)
		    pv[ply-1][j] = pv[ply][j];
		  pv_length[ply-1] = pv_length[ply];

		  post_fh_thinking(root_score, &moves[i]);
		}
	      else if (root_score <= alpha)
		{
		  /* update the pv: */
		  /* maybe not..fail low yields nonsense */
		  /*		  pv[ply-1][ply-1] = moves[i];
		   for (j = ply; j < pv_length[ply]; j++)
		     pv[ply-1][j] = pv[ply][j];
		     pv_length[ply-1] = pv_length[ply];*/

		  post_fl_thinking(root_score, &moves[i]);
		}
	      else
		{
		  /* update the pv: */
		  pv[ply-1][ply-1] = moves[i];
		  for (j = ply; j < pv_length[ply]; j++)
		    pv[ply-1][j] = pv[ply][j];
		  pv_length[ply-1] = pv_length[ply];

		  post_thinking(root_score);
		}
	    }
	}
      else
	{
	  root_score = -search (-alpha-1, -alpha, depth-1, FALSE);
	  ep_square = old_ep;

	  if ((root_score > alpha) && (root_score < beta) && !time_exit)
	    {
	      if (i_depth >= mindepth)
	      	{
		  /* update the pv: */
		  pv[ply-1][ply-1] = moves[i];
		  for (j = ply; j < pv_length[ply]; j++)
		    pv[ply-1][j] = pv[ply][j];
		  pv_length[ply-1] = pv_length[ply];

		  /*  if (xb_mode) printf("++\n"); */
		  post_fail_thinking(root_score, &moves[i]); 
		}
	      
	      root_score = -search(-beta, -root_score, depth-1, FALSE);
	      ep_square = old_ep;
	    }
	}

      if (root_score > cur_score) 
	{
	  cur_score = root_score;
	  bestmovenum = i;
	  best_move = moves[i];
	}
      
      /* check to see if we've aborted this search before we found a move: */
      /* or a failed search */   
      if (time_exit && (no_moves || cur_score <= originalalpha || cur_score >= originalbeta))
	{
	  time_failure = TRUE;
	}
      
      no_moves = FALSE;
      legal_move = TRUE;
    }

    unmake (&moves[0], i);
    ply--;

    /* if we've run out of time, return the best we have so far: */
    if (time_exit)
      return best_move;

    /* check our current score vs. alpha: */
    if (root_score > alpha && legal_move) {

      /* we have a cutoff, so update our killers: */
      if (root_score > killer_scores[ply]) {
	killer_scores2[ply] = killer_scores[ply];
	killer_scores[ply] = root_score;
	killer3[ply] = killer2[ply];
	killer2[ply] = killer1[ply];
	killer1[ply] = moves[i];
      }
      else if (root_score > killer_scores2[ply]) {
	killer_scores2[ply] = root_score;
	killer3[ply] = killer2[ply];
	killer2[ply] = moves[i];
      }

      /* update the history heuristic since we have a cutoff: */
      /* PGC square it */
      history_h[moves[i].from][moves[i].target] += depth * depth;

      alpha = root_score;
      best_move = moves[i];
      bestmovenum = i;
      cur_score = alpha;

      /* update the pv: */
      pv[ply][ply] = moves[i];
      for (j = ply+1; j < pv_length[ply+1]; j++)
	pv[ply][j] = pv[ply+1][j];
      pv_length[ply] = pv_length[ply+1];

      if (cur_score >= beta) return best_move;

      /* print out thinking information: */
      if (post && i_depth >= mindepth) {
	post_thinking (alpha);
      }
    }
    if (legal_move)
      first = FALSE;
  }

  /* check to see if we are mated / stalemated: */
  if (no_moves) {
    if (in_check ()) {
      if (white_to_move == 1) {
	result = white_is_mated;
      }
      else {
	result = black_is_mated;
      }
    }
    else {
      result = stalemate;
    }
  }

  /* check to see if we have mated our opponent: */
  if (root_score == INF-2) {   /* root_score */
    if (white_to_move == 1) {
      result = black_is_mated;
    }
    else {
      result = white_is_mated;
    }
  }

  return best_move;

}


move_s think (void) {

  /* Perform iterative deepening to go further in the search */
  
  move_s comp_move, temp_move;
  int ep_temp, i, j;
  long int elapsed, temp_score, true_score;
  char postmove[STR_BUFF];
  clock_t cpu_start, cpu_end; 
  float et = 0;
  int alpha, beta;
  int tmptmp;

  nodes = 0;
  qnodes = 0;
  ply = 0;

  ECacheProbes = 0;
  ECacheHits = 0;
  TTProbes = 0;
  TTHits = 0;
  TTStores = 0;  
  NCuts = 0;
  NDCuts = 0;
  NTries = 0;
  NDTries = 0;
  TExt = 0;
  DeltaTries = 0;
  DeltaCuts = 0;
  FH = 0;
  FHF = 0;
  PVS = 0;
  FULL = 0;
  PVSF = 0;
  ext_check = 0;
  razor_drop = 0;
  razor_material = 0;

  true_i_depth = 0;
  bestmovenum = -1;

  ep_temp = ep_square;
 
  start_time = rtime ();
  cpu_start = clock ();
  
   /* before we do anything, check to see if we can make a move from book! */
   if (book_ply < 20 && !is_analyzing && !is_pondering) {
     comp_move = choose_book_move();
      /* if choose_book_move() didn't return a junk move indicating that
         no book move was found, play the book move! :) */

      if (comp_move.target != 0) {
	comp_to_coord (comp_move, postmove);
	printf("0 0 0 0 %s (Book Move)\n", postmove);
	cpu_end = clock ();
	
	ep_square = ep_temp;

	return comp_move;
      }
   }

  /* allocate our time for this move: */

   if (!is_pondering)
     {
       if (!fixed_time)
	 {
	   if (go_fast)
	     {
	       tmptmp = allocate_time();
	       if (tmptmp > 70)
		 time_for_move = 70;
	       else
		 time_for_move = tmptmp;
	     }
	   else
	     {
	       time_for_move = allocate_time ();
	     }	
	 }
       else
	 {
	   time_for_move = fixed_time;
	 }
     }
   else
     {
       time_for_move = 999999;
     };

   if (time_for_move > 100)
     LoadLearn();
   
   /* clear the pv before a new search: */
  for (i = 0; i < PV_BUFF; i++)
    for (j = 0; j < PV_BUFF; j++)
      pv[i][j] = dummy;

  /* clear the history heuristic: */
  memset (history_h, 0, sizeof (history_h));

  /* clear the killer moves: */
  for (i = 0; i < PV_BUFF; i++) {
    killer_scores[i] = -INF;
    killer_scores2[i] = -INF;
    killer1[i] = dummy;
    killer2[i] = dummy;
    killer3[i] = dummy;
  }

  cpu_start = clock();

  temp_score = 0;
  cur_score = 0;
  true_score = 0;

  for (i_depth = 1; i_depth <= maxdepth; i_depth++) {

    /* don't bother going deeper if we've already used 2/3 of our time, and we
       haven't finished our mindepth search, since we likely won't finsish */
    elapsed = rdifftime (rtime (), start_time);
    if (elapsed > time_for_move*2.0/3.0 && i_depth > mindepth)
      break;

    alpha = temp_score - (Variant == Normal ? 50 : 100);
    beta = temp_score + (Variant == Normal ? 50 : 100);

    ep_square = ep_temp;
    temp_move = search_root (alpha, beta, i_depth);

    if (cur_score <= alpha && !time_exit) /* fail low */
      {
	alpha = cur_score - (Variant == Normal ? 350 : 600);
	beta = cur_score + 1;

	printf("RS\n");

	ep_square = ep_temp;
	temp_move = search_root (alpha, beta, i_depth);	

	if (cur_score <= alpha && !time_exit)
	  {
	    alpha = -(INF+1);
	    beta = cur_score + 1;

	    ep_square = ep_temp;
	    temp_move = search_root (alpha, beta, i_depth);	
	  }
	else if (cur_score >= beta && !time_exit)
	  {
	    /*printf("Fail high on fail-low\n");*/
	  }
      }
    else if (cur_score >= beta && !time_exit) /* fail high */
      {
	alpha = cur_score - 1;
	beta = cur_score + (Variant == Normal ? 350 : 600);

	printf("RS\n");

	ep_square = ep_temp;
	temp_move = search_root (alpha, beta, i_depth);

	if (cur_score >= beta && !time_exit)
	  {
	    alpha = cur_score - 1;
	    beta = +(INF+1);
	    
	    printf("RS\n");
	    
	    ep_square = ep_temp;
	    temp_move = search_root (alpha, beta, i_depth);
	  }
	else if (cur_score <= alpha && !time_exit)
	  {
	    /*printf("Fail low on fail-high\n");*/
	  };
      };

    if (interrupt()) 
      {
	ep_square = ep_temp;
	if (is_pondering)
	  return;
	else if (!go_fast)
	  break;
      }

    /* if we haven't aborted our search on time, set the computer's move
       and post our thinking: */
    if (!time_failure && !time_exit) {
      /* if our search score suddenly drops, and we ran out of time on the
	 search, just use previous results */
      /* GCP except when we found a mate...maybe generalise ? */
      /* disabled for now */
      /*   if (time_exit && (cur_score < temp_score-40) && (cur_score > -900000))
      	break;*/

      true_i_depth = i_depth;

      comp_move = temp_move;
      temp_score = cur_score;
      if (i_depth >= mindepth)
	post_thinking (cur_score);
      
    }

    /* reset the killer scores (we can keep the moves for move ordering for
       now, but the scores may not be accurate at higher depths, so we need
       to reset them): */
    for (j = 0; j < PV_BUFF; j++) {
      killer_scores[j] = -INF;
      killer_scores2[j] = -INF;
    }

  }

  ep_square = ep_temp;

  cpu_end = clock();

  et = (cpu_end-cpu_start)/(double) CLOCKS_PER_SEC;

  /* update our elapsed time_cushion: */
  if (moves_to_tc) {
    elapsed = rdifftime (rtime (), start_time);
    time_cushion += time_for_move-elapsed+inc;
  }

  if (post && xb_mode && !is_pondering && 
	result != black_is_mated &&
	result != white_is_mated &&
	result != stalemate)
    {
      if (temp_score > 900000) 
	{
	  printf("tellics kibitz Mate in %d\n", (int)((1000000-temp_score)/2));
	}
      
      comp_to_coord (comp_move, postmove);

      if ((et > 0) && (Variant != Bughouse))
	{
	  printf("tellics whisper ply: %d score: %ld move: %s nodes: %ld qp: %.0f%% fh: %.0f%% pvs: %.0f%% pvsf: %.0f%% time: %.2f nps: %ld\n",
		 true_i_depth, temp_score, postmove, nodes, 
		 (((float)qnodes*100)/((float)nodes+1)),
		 ((float)FHF*100)/((float)FH+1),
		 ((float)PVS*100)/((float)FULL+1),
		 ((float)PVSF*100)/((float)PVS+1),
		 et, 
		 (long)((float) nodes/(float) (et)));
	}
    }

  
  if ((result != white_is_mated) 
      && (result != black_is_mated)
      && (result != stalemate) && (true_i_depth >= 2))
    {
      if (bestmovenum == -1) DIE;

	Learn(temp_score, bestmovenum, true_i_depth);
    }

  if ((temp_score < -999996) && (Variant == Bughouse))
    {
      must_sit = TRUE;

      /* shut up if the mate is already played */
      if (temp_score > -1000000)
	{
	  if (partnerdead)
	    {
	      printf("tellics kibitz Both players dead...resigning...\n");
	      printf("tellics resign\n");
	    }
	  else
	    {
	      printf("tellics ptell I'll have to sit...(dead)\n");
	    }
	}
    }
  else if ((temp_score > -60000) && (temp_score < -40000) && (Variant == Bughouse) && !partnerdead)
    {
      must_sit = TRUE;
      printf("tellics ptell I'll have to sit...(piece)\n");
    }

  return comp_move;

}


void tree (int depth, int indent, FILE *output, char *disp_b) {

  move_s moves[MOVE_BUFF];
  int num_moves, i, j, ep_temp;

  ep_temp = ep_square;
  num_moves = 0;

  /* return if we are at the maximum depth: */
  if (!depth) {
    return;
  }

  /* generate the move list: */
  gen (&moves[0], &num_moves);

  /* loop through the moves at the current depth: */
  for (i = 0; i < num_moves; i++) {
    make (&moves[0], i);

    /* check to see if our move is legal: */
    if (check_legal (&moves[0], i)) {
      /* indent and print out our line: */
      for (j = 0; j < indent; j++) {
	fputc (' ', output);
      }
      print_move (&moves[0], i, output);
      fprintf (output, "\n");

      /* display board if desired: */
      if (disp_b[0] == 'y')
	display_board (output, 1);

      /* go deeper into the tree recursively, increasing the indent to
	 create the "tree" effect: */
      tree (depth-1, indent+2, output, disp_b);
    }

    /* unmake the move to go onto the next: */
    unmake(&moves[0], i);
  }

  ep_square = ep_temp;

}
