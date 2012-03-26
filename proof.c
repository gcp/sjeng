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

   File: proof.c                                        
   Purpose: contains functions related to the pn-search

 */

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"
#include "limits.h"

#define FALSE 0
#define TRUE 1
#define UNKNOWN 2
#define STALEMATE 3		/* special case because pn-search only assumes 1/0 */

#define PN_INF 100000000

#define MAXSEARCH 250000
/* we can exceed MAXSEARCH before exiting the main search loop */
#define SAFETY      5000

int nodecount;
int nodecount2;
int pn2;
long long frees;
int iters;
int maxply;
int ply;
int pn_time;
move_s pn_move;
move_s pn_saver;

bool kibitzed;

typedef struct node
  {
    unsigned char value;
    unsigned char num_children;
    unsigned char expanded;
    unsigned char evaluated;
    int proof;
    int disproof;
    struct node **children;
    struct node *parent;
    move_s move;
  }
node_t;

void pn2_eval (node_t *node);
void suicide_pn_eval (node_t *this);
void std_pn_eval (node_t *this);

unsigned long p_rep_list[8192];

unsigned char *membuff;
int bufftop = 0;

void* Xmalloc(int size)
{
  int oldtop = bufftop;
  
  bufftop += size;
  
  return (&membuff[oldtop]);
};

void Xfree(void)
{
  bufftop = 0;
};

void freenodes (node_t * node)
{
  int i;

  if (!node)
    return;

  if (node->children)
    {
      if (node->num_children > 0)
	{
	  for (i = 0; i < (node->num_children); i++)
	    {
	      if (node->children[i] != 0)
		{
		  freenodes (node->children[i]);
		};
	    };
	  free (node->children);
	}
    };

  free (node);
};

void pn_eval(node_t * this)
{
  if (Variant == Suicide)
    {    
      suicide_pn_eval(this);
    }
 else 
   {
     std_pn_eval(this);
   }
}

void std_pn_eval (node_t * this)
{
  int ep_temp;
  int num_moves;
  move_s moves[MOVE_BUFF];
  int mate;
  int i;

  this->evaluated = TRUE;

  ep_temp = ep_square;

  if ((white_to_move && is_attacked (wking_loc, WHITE))
      || (!white_to_move && is_attacked (bking_loc, BLACK)))
    {

      num_moves = 0;
      gen (&moves[0]);
      num_moves = numb_moves;

      mate = TRUE;

      for (i = 0; i < num_moves; i++)
	{
	  make (&moves[0], i);

	  /* check to see if our move is legal: */
	  if (check_legal (&moves[0], i))
	    {
	      mate = FALSE;
	      unmake (&moves[0], i);
	      break;
	    };

	  unmake (&moves[0], i);
	}

      if (mate == TRUE)
	{
	  /* proven or disproven */
	  if (ToMove == root_to_move)
	    {
	      /* root mover is mated-> disproven */
	      this->value = FALSE;
	    }
	  else
	    {
	      this->value = TRUE;
	    };
	}
      else
	{
	  this->value = UNKNOWN;
	};
    }
  else
    {
      this->value = UNKNOWN;
    };

  ep_square = ep_temp;

};

void suicide_pn_eval(node_t *this)
{
  int j, a, i;
  int wp = 0, bp = 0;

  this->evaluated = TRUE;
  
  for (j = 1, a = 1; (a <= piece_count); j++) 
    {
      i = pieces[j];
      
      if (!i)
	continue;
      else
	a++;
      
      switch (board[i])
	{
	case wpawn:
	case wbishop:
	case wrook:
	case wking:
	case wqueen:
	case wknight: wp++; break;
	case bpawn:
	case bbishop:
	case brook:
	case bking:
	case bqueen:
	case bknight: bp++; break;
	}
    }
  
  if (!wp)
    {
      /* white has no pieces */
      /* proven or disproven */
      if (!root_to_move)
	{
	  /* root mover is mated-> proven */
	  this->value = TRUE;
	}
      else
	{
	  this->value = FALSE;
	};
    }
  else if (!bp)
    {
      /* black has no pieces */
      if (!root_to_move)
	{
	  /* root mover is mated-> disproven */
	  this->value = FALSE;
	}
      else
	{
	  this->value = TRUE;
	};
    }
  else
    {
      this->value = UNKNOWN;
    };  
};

node_t *select_most_proving (node_t * node)
{
  int i;
  node_t *tnode;

  tnode = node;

  while (tnode->expanded)
    {
      if (ToMove == root_to_move)
	{
	  i = 0;

	  while (tnode->children[i]->proof != tnode->proof)
	    {
	      i++;
	    };
	}
      else
	{
	  i = 0;

	  while (tnode->children[i]->disproof != tnode->disproof)
	    {
	      i++;
	    };
	};

      tnode = tnode->children[i];

      p_rep_list[ply] = hash;
      
      make (&tnode->move, 0);
      ply++;
      

      if (ply > maxply)
	maxply = ply;

    };

  return tnode;

};

void set_proof_and_disproof_numbers (node_t * node)
{
  int proof;
  int disproof;
  int i;
  move_s moves[MOVE_BUFF];
  int l, num_moves;
  int ept;
  int reploop;

  if (node->expanded)
    {
      if (ToMove != root_to_move)
	{
	  proof = 0;
	  disproof = +PN_INF;

	  for (i = 0; i < node->num_children; i++)
	    {
	      proof += node->children[i]->proof;

	      if (proof > PN_INF)
		proof = PN_INF;

	      if (node->children[i]->disproof < disproof)
		{
		  disproof = node->children[i]->disproof;
		}
	    }
	}
      else
	{
	  disproof = 0;
	  proof = +PN_INF;

	  for (i = 0; i < node->num_children; i++)
	    {

	      disproof += node->children[i]->disproof;

	      if (disproof > PN_INF)
		disproof = PN_INF;

	      if (node->children[i]->proof < proof)
		{
		  proof = node->children[i]->proof;
		}
	    }
	}

      node->proof = proof;
      node->disproof = disproof;

    }
  else if (node->evaluated)
    {
      if (node->value == UNKNOWN)
	{

	  for (reploop = 0; reploop < ply; reploop++)
	  {
	    if (p_rep_list[reploop] == hash)
	    {
	      node->proof = 5000;
	      node->disproof = 5000;
	      return;
	    };
	  };
	  
	  ept = ep_square;

	  num_moves = 0;
	  gen (&moves[0]);
	  num_moves = numb_moves;

	  if (Variant != Suicide)
	    {
	      l = 0;

	      for (i = 0; i < num_moves; i++)
		{
		  make (&moves[0], i);
		  /* check to see if our move is legal: */
		  if (check_legal (&moves[0], i))
		    {
		      l++;
		    }
		  unmake (&moves[0], i);
		};
	    }
	  else
	    {
	      l = numb_moves;
	    };

	  if (l == 0)
	    {
	      /* might be stalemate too */
	      node->proof = 1;
	      node->disproof = 1;
	    }
	  else if (ToMove == root_to_move)	/* OR */
	    {
	      if (Variant != Suicide)
		{
		  node->proof = 1 + ((ply + 1) / 6);
		  node->disproof = l + ((ply + 1) / 6);
		}
	      else
		{
		  node->proof = 1;
		  node->disproof = l;
		}
	    }
	  else
	    {
	      if (Variant != Suicide)
		{
		  node->proof = l + ((ply + 1) / 6);
		  node->disproof = 1 + ((ply + 1) / 6);
		}
	      else
		{
		  node->proof = l;
		  node->disproof = 1;
		}
	    }

	  ep_square = ept;
	}
      else if (node->value == FALSE)
	{
	  node->proof = +PN_INF;
	  node->disproof = 0;
	}
      else if (node->value == TRUE)
	{
	  node->proof = 0;
	  node->disproof = +PN_INF;
	}
      else if (node->value == STALEMATE)
	{
	  /* don't look at this node, its a dead-end */
	  node->proof = 5000;
	  node->disproof = 5000;
	};
    }
  else
    {
      node->proof = node->disproof = 1;
    }
}

void develop_node (node_t * node)
{
  int num_moves;
  move_s moves[MOVE_BUFF];
  int i, l;
  node_t *newnode;
  node_t **newchildren;
  int ept;

  ept = ep_square;

//  if (!pn2)
//    pn2_eval(node);

  num_moves = 0;
  gen (&moves[0]);
  num_moves = numb_moves;
  
//  if (pn2)
    node->children = (node_t **) Xmalloc (num_moves * sizeof (node_t **));
//  else
//    newchildren = (node_t **) malloc (num_moves * sizeof (node_t **));
    
  l = 0;

  for (i = 0; i < num_moves; i++)
    {
      make (&moves[0], i);

      /* check to see if our move is legal: */
      if (check_legal (&moves[0], i))
	{
  //        if (pn2)
	    newnode = (node_t *) Xmalloc (sizeof (node_t));
//	  else
//	    newnode = (node_t *) malloc (sizeof (node_t));

	  newnode->value = 0;

//	  if (!pn2)
//	    { 
//	      newnode->proof = node->children[l]->proof;
//	      newnode->disproof = node->children[l]->disproof;
//	    }
//	  else
//	    {
	      newnode->proof = newnode->disproof = 1; 
//	    };

	  newnode->num_children = 0;
	  newnode->parent = node;
	  newnode->evaluated = FALSE;
	  newnode->expanded = FALSE;
	  newnode->move = moves[i];

//	  if (!pn2)
//	    newchildren[l] = newnode;
//	  else 
	    node->children[l] = newnode;	  	  

	  l++;

//	  if (pn2 == FALSE)
//	    /*use delayed eval */;
//	  else if (pn2)
	    pn_eval (newnode);

//	  if (pn2)
	    set_proof_and_disproof_numbers (newnode);

	  unmake (&moves[0], i);	 

	}
      else
	unmake (&moves[0], i);
    };

  node->expanded = TRUE;
  node->num_children = l;
  
//  if (!pn2)
//    node->children = newchildren;

  /* account for stalemate ! */
  if (node->num_children == 0)
    {
      node->expanded = FALSE;
      node->evaluated = TRUE;
      node->value = STALEMATE;
    };

//  if (pn2)
//    nodecount2 += num_moves;
//  else
    nodecount += num_moves;

  frees += num_moves;
  
  ep_square = ept;

  //if (!pn2) Xfree();
};

void update_ancestors (node_t * node)
{
  node_t *tnode, *prevnode;
  
  tnode = node;
  prevnode = node;

  while (tnode != 0)
    {
      set_proof_and_disproof_numbers (tnode);

      prevnode = tnode;

      if (tnode->move.target != 0)
	{			/* traverse */
	  unmake (&tnode->move, 0);
	  ply--;
	}

      tnode = tnode->parent;
    };

  if (prevnode->move.target != 0)
    {
      make (&prevnode->move, 0);
      ply++;
    }

  return;

};

void 
pn2_eval (node_t * root)
{
  node_t *mostproving;
  node_t *newroot;
  node_t *currentnode;
  node_t *oldparent;

  nodecount2 = 0;
  pn2 = TRUE;

  oldparent = root->parent;
  root->parent = 0;

  pn_eval (root);

  set_proof_and_disproof_numbers (root);

  currentnode = root;

  while (root->proof != 0 && root->disproof != 0 && nodecount2 < nodecount
    )
    {
      mostproving = select_most_proving (root);
      develop_node (mostproving);
      update_ancestors (mostproving);
    };

  root->expanded = FALSE;
  root->num_children = 0;

  root->parent = oldparent;

  pn2 = FALSE;
  
};


void 
proofnumbersearch (void)
{
  node_t *root;
  node_t *mostproving;
  node_t *currentnode;
  rtime_t start_time;
  char output[8192];
  char PV[8192];
  int i;
  int eps;
  int bdp;

  nodecount = 1;
  iters = 0;
  frees = 0;
  ply = 1;
  maxply = 0;

  root_to_move = ToMove;
  
  eps = ep_square;

  start_time = rtime ();

  root = (node_t *) calloc (1, sizeof (node_t));

  membuff = (unsigned char *) calloc(MAXSEARCH + SAFETY, sizeof(node_t));

  pn_eval (root);

  set_proof_and_disproof_numbers (root);

  currentnode = root;

  while (root->proof != 0 && root->disproof != 0 && nodecount < MAXSEARCH)
    {
      mostproving = select_most_proving (currentnode);
      develop_node (mostproving);
      update_ancestors (mostproving);

      iters++;

      if ((iters % 64) == 0)
	{
	  //printf("P: %d D: %d N: %d S: %Ld Mem: %2.2fM Iters: %d ", root->proof, root->disproof, nodecount, frees, (((nodecount) * sizeof(node_t) / (float)(1024*1024))), iters);
	  
#ifdef SHOWPVS
	  printf ("PV: ");
	  
	  memset (output, 0, sizeof (output));
	  memset (PV, 0, sizeof (PV));
	  //currentnode = root;
	  ply = 0;
	  
	  while (currentnode->expanded)
	    {
	      if (ToMove == root_to_move)
		{
		  i = 0;
		  while (currentnode->children[i]->proof != currentnode->proof)
		    {
		      i++;
		    };
		}
	      else
		{
		  i = 0;
		  while (currentnode->children[i]->disproof != currentnode->disproof)
		    {
		      i++;
		    }
		};
	      
	      currentnode = currentnode->children[i];
	      
	      comp_to_coord (currentnode->move, output);
	      printf ("%s ", output);
	      strcat (PV, output);
	      strcat (PV, " ");
	      
	      make (&currentnode->move, 0);
	      
	      ply++;
	    };
	  
	  while (currentnode != root)
	    {
	      unmake (&currentnode->move, 0);
	      currentnode = currentnode->parent;
	    };
#endif
//	  printf("\n");
       
      	  if ((rdifftime (rtime (), start_time) > pn_time))
       	    break;
	}
    };
  
  printf ("P: %d D: %d N: %d S: %Ld Mem: %2.2fM Iters: %d\n", root->proof, root->disproof, nodecount, frees, (((nodecount) * sizeof (node_t) / (float) (1024 * 1024))), iters);

  if (xb_mode && post)
    printf ("tellics whisper proof %d, disproof %d, %d nodes, %d iters, highest depth %d\n", root->proof, root->disproof, nodecount, iters, maxply);
  
  while (currentnode != root)
    {
      unmake (&currentnode->move, 0);
      currentnode = currentnode->parent;
    };

  if (root->proof == 0)
    {
      root->value = TRUE;

      printf ("This position is WON.\n");
      printf ("PV: ");

      memset (output, 0, sizeof (output));
      memset (PV, 0, sizeof (PV));
      //currentnode = root;
      ply = 0;

      while (currentnode->expanded)
	{
	  if (ToMove == root_to_move)
	    {
	      i = 0;
	      while (currentnode->children[i]->proof != currentnode->proof)
		{
		  i++;
		};
	    }
	  else
	    {
	      i = 0;
	      while (currentnode->children[i]->disproof != currentnode->disproof)
		{
		  i++;
		}
	    };

	  currentnode = currentnode->children[i];

	  comp_to_coord (currentnode->move, output);
	  printf ("%s ", output);
	  strcat (PV, output);
	  strcat (PV, " ");

	  make (&currentnode->move, 0);

	  if (ply == 0)
	    pn_move = currentnode->move;

	  ply++;
	};

      while (currentnode != root)
	{
	  unmake (&currentnode->move, 0);
	  currentnode = currentnode->parent;
	};

      if (!kibitzed && xb_mode)
	{
	  kibitzed = TRUE;
	  printf ("\ntellics kibitz Forced win in %d moves.\n", (ply+1)/2);
	}

      printf ("\n");
    }
  else if (root->disproof == 0)
    {
      root->value = FALSE;
      printf ("This position is LOST.\n");
      
      pn_move = dummy;
    }
  else
    {
      root->value = UNKNOWN;
      printf ("This position is UNKNOWN.\n");

      pn_move = dummy;
    };

  /* find the move which is least likely to lose */
  bdp = -1;
  
  for (i = 0; i < root->num_children; i++)
    {
      if (root->children[i]->disproof > bdp)
	{
	  bdp = root->children[i]->disproof;
	  pn_saver = root->children[i]->move;
	}
    };

  free(root);
  Xfree();
  free(membuff);
  
  ep_square = eps;

  return;
}

move_s proofnumbercheck(move_s compmove)
{
  node_t* root;
  node_t *mostproving;
  node_t *currentnode;
  rtime_t start_time;
  int i;
  int eps;
  move_s resmove;
  
  nodecount = 0;
  iters = 0;
  frees = 0;
  ply = 1;
  maxply = 0;
  
  /* make our move to check */
  make(&compmove, 0);

  root_to_move = ToMove;
  
  eps = ep_square;
  
  start_time = rtime();
  
  root = (node_t *) calloc(1, sizeof(node_t));

  membuff = (unsigned char *) calloc(MAXSEARCH, sizeof(node_t));
  
  pn_eval(root);

  set_proof_and_disproof_numbers(root);

  currentnode = root;

  while (root->proof != 0 && root->disproof != 0 && nodecount < MAXSEARCH)
    {
      mostproving = select_most_proving(currentnode);
      develop_node(mostproving);
      update_ancestors(mostproving);
	
      iters++;
      
      if ((iters % 64) == 0)
	{
	  //	 printf("P: %d D: %d N: %d S: %d Mem: %2.2fM Iters: %d\n", root->proof, root->disproof, nodecount, frees, (((nodecount) * sizeof(node_t) / (float)(1024*1024))), iters);
	  if ((rdifftime (rtime (), start_time) > pn_time))
	    break;
	}
    };

  printf("P: %d D: %d N: %d S: %d Mem: %2.2fM Iters: %d\n", root->proof, root->disproof, nodecount, frees, (((nodecount) * sizeof(node_t) / (float)(1024*1024))), iters);

  while(currentnode != root)
  {
    unmake(&currentnode->move, 0);
    currentnode = currentnode->parent;
  };  

  unmake(&compmove, 0);
  
  if (root->proof == 0)
    {
      if (xb_mode)
	printf("tellics whisper Panic move!\n");
      
      /* ok big problem our ab move loses */
      root->value = TRUE;

      /* use best disprover instead */
      resmove = pn_saver;
	         
    }
  else if (root->disproof == 0)
    {
      /* ab move wins...unlikely due to earlier pnsearch */

      root->value = FALSE;
      resmove = compmove;

    }
  else
    {
      root->value = UNKNOWN;
      resmove = compmove;

    };

  Xfree();
  free(root);
  free(membuff);

  ep_square = eps;
  
  return resmove;
}












