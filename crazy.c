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
                                                          
    File: crazy.c
    Purpose: bughouse/crazyhouse specific functions                  

*/ 

#include <assert.h>
#include "sjeng.h"
#include "protos.h"
#include "extvars.h"

int holding[2][16];
int num_holding[2];

int drop_piece;

int hand_eval;

unsigned long hold_hash;

#define HHash(x,y)  (hold_hash ^= zobrist[(x)][(y)])

/* input example : holding [BPPP] [QR] */
/* based on db's parser */
void ProcessHoldings(char str[])
{
  int c, i;

  i = 0;

  memset(holding, 0, sizeof(holding));

  hand_eval = 0;

  num_holding[WHITE] = 0;
  num_holding[BLACK] = 0;

  for(c = WHITE; c <= BLACK; c++) 
    {
      while(str[i++] != '[')
	if(str[i] == 0) return;
      
      while(str[i] != ']') {
	switch(str[i++]) {
	case 'p':
	case 'P':
	  holding[c][c == WHITE ? wpawn : bpawn]++;
	  num_holding[c]++;
	  HHash((c == WHITE ? wpawn : bpawn),
		holding[c][(c == WHITE ? wpawn : bpawn)]); 
	  break;
	case 'q':
	case 'Q':
	  holding[c][c == WHITE ? wqueen : bqueen]++;
	  num_holding[c]++;
	  HHash((c == WHITE ? wqueen : bqueen),
		holding[c][(c == WHITE ? wqueen : bqueen)]); 
	  break;
	case 'r':
	case 'R':
	  holding[c][c == WHITE ? wrook : brook]++;
	  num_holding[c]++;
	  HHash((c == WHITE ? wrook : brook),
		holding[c][(c == WHITE ? wrook : brook)]); 
	  break;
	case 'b':
	case 'B':
	  holding[c][c == WHITE ? wbishop : bbishop]++;
	  num_holding[c]++;
	  HHash((c == WHITE ? wbishop : bbishop),
		holding[c][(c == WHITE ? wbishop : bbishop)]); 
	  break;
	case 'n':
	case 'N':
	  holding[c][c == WHITE ? wknight : bknight]++;
	  num_holding[c]++;
	  HHash((c == WHITE ? wknight : bknight),
		holding[c][(c == WHITE ? wknight : bknight)]); 
	  break;
	default:
	  return;
	}
      }
    }
}


int text_to_piece(char txt, int who)
{
  switch(txt)
    {
    case 'p':
    case 'P':
      return (who == WHITE ? wpawn : bpawn);
    case 'b':
    case 'B':
      return (who == WHITE ? wbishop : bbishop);
    case 'n':
    case 'N':
      return (who == WHITE ? wknight : bknight);
    case 'r':
    case 'R':
      return (who == WHITE ? wrook : brook);
    case 'q':
    case 'Q':
      return (who == WHITE ? wqueen : bqueen);
    };

  return npiece;
}

int SwitchColor(int piece)
{
  int t[] = { 0, bpawn, wpawn, bknight, wknight, 0, 0, brook, wrook, bqueen, wqueen, bbishop, wbishop };

  assert(piece > frame  && piece < npiece);

  return(t[piece]);
}

int SwitchPromoted(int piece)
{
  int t[] = { 0, bpawn, wpawn, bpawn, wpawn, 0, 0, bpawn, wpawn, bpawn, wpawn, bpawn, wpawn };

  assert(piece > frame && piece < npiece);

  return(t[piece]);
}

void addHolding(int what, int who)
{

  if (Variant == Crazyhouse)
    {

      holding[who][what]++;
      
      num_holding[who]++;
      
      HHash(what, holding[who][what]);

    };

  hand_eval += hand_value[what]; 

  Material += material[what];

  return;
    
}

void removeHolding(int what, int who)
{

  if (Variant == Crazyhouse)
    {

      assert(holding[who][what] > 0);
      assert(holding[who][what] < 10);	
      
      HHash(what, holding[who][what]);
      
      holding[who][what]--;
      
      num_holding[who]--;
      
    }

  hand_eval -= hand_value[what];

  Material -= material[what];

  return;

}

void DropaddHolding(int what, int who)
{
  holding[who][what]++;
  
  num_holding[who]++;
  
  HHash(what, holding[who][what]);
  
  hand_eval += hand_value[what]; 

  Material += material[what];

  return;
}

void DropremoveHolding(int what, int who)
{
  assert(holding[who][what] > 0);

  assert(holding[who][what] < 10);	

  HHash(what, holding[who][what]);
  
  holding[who][what]--;
  
  num_holding[who]--;
  
  hand_eval -= hand_value[what];

  Material -= material[what];

  return;
}

void printHolding(void)
{

  printf("WP: %d WR: %d WB: %d WN: %d WQ: %d\n",
	 holding[WHITE][wpawn], holding[WHITE][wrook], 
	 holding[WHITE][wbishop],
	 holding[WHITE][wknight], holding[WHITE][wqueen]);
  
  printf("BP: %d BR: %d BB: %d BN: %d BQ: %d\n",
	 holding[BLACK][bpawn], holding[BLACK][brook], 
	 holding[BLACK][bbishop],
	 holding[BLACK][bknight], holding[BLACK][bqueen]);

}
