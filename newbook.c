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

    File: newbook.c                                             
    Purpose: general function concerning the binary hashed book

*/

#include "sjeng.h"
#include "protos.h"
#include "extvars.h"
#include "gdbm.h"
#include <sys/stat.h>

#define BUILDTHRESHOLD 2
#define PLAYTHRESHOLD 5

typedef struct 
{
  unsigned long hashkey;
} hashkey_t;

typedef struct 
{
  unsigned long played;
} posinfo_t;

typedef struct 
{
  int result; /* 0: 1-0  1:1/2  2:0-1  3:? */
} pgn_header_t;

unsigned long kksize;
unsigned char *keycache;

unsigned long lastbookpos, lastbooktomove;

void get_header(FILE *pgnbook, pgn_header_t *pgn_header)
{
  int ch;
  char buff[STR_BUFF];
  int b;
  int terminate = FALSE;
  
  memset(pgn_header, 0, sizeof(pgn_header_t));
  
  while(!terminate)
    {
      ch = getc(pgnbook);
      
      if (ch == EOF)
	break;
      
      /* beginning of a header field */
      if (ch == '[')
	{
	  b = 0;
	  memset(buff, 0, sizeof(buff));
	  
	  while(((buff[b++] = getc(pgnbook)) != ']') && (b < STR_BUFF));
	  buff[--b] = '\0';
	  
	  /* buff now contains the field, minus the [] */
	  /* file position is just after ] */
	  
	  //printf ("Read header: -%s-\n", buff);
	  
	  if (!strncmp("Result", buff, 6))
	    {
	      if (strstr(buff+6, "1-0"))
		pgn_header->result = 0;
	      else if (strstr(buff+6, "1/2-1/2"))
		pgn_header->result = 1;
	      else if (strstr(buff+6, "0-1"))
		pgn_header->result = 2;
	      else if (strstr(buff+6, "*"))
		pgn_header->result = 3;
	    }
	}
      /* space or newlines between headers */
      else if (ch == ' ' || ch == '\n' || ch == '\r'); 
      else  /* no more headers, put back last char */
	{
	  //printf("End of header: -%c-\n", ch);
	  terminate = TRUE;
	  ungetc(ch, pgnbook);
	}
    }
}

void add_current(GDBM_FILE binbook, pgn_header_t pgn_header)
{
  hashkey_t key;
  posinfo_t posinfo;
  posinfo_t *pst;
  datum index;
  datum data;
  int win = 0, loss = 0;
  int ret;
  
  /* fill in the key field */
  key.hashkey = (hash ^ ToMove);
  
  if (keycache[key.hashkey % kksize] >= BUILDTHRESHOLD)
    {
      
      index.dptr = (char*) &key;
      index.dsize = sizeof(key);
      
      posinfo.played = 2;
      
      data.dptr = (char*) &posinfo;
      data.dsize = sizeof(posinfo);
      
      ret = gdbm_store(binbook, index, data, GDBM_INSERT);
      
      if (ret == 1)
	{
	  data = gdbm_fetch(binbook, index);
	  
	  pst = data.dptr;
	  pst->played++;
	  
	  gdbm_store(binbook, index, data, GDBM_REPLACE);
	  
	  free(data.dptr);
	}
    }
  else
    keycache[key.hashkey % kksize]++;
  
}

void replay_game(FILE *pgnbook, GDBM_FILE binbook, pgn_header_t pgn_header)
{
  int ch, xch;
  char movebuff[STR_BUFF], sjmove[STR_BUFF];
  int ms;
  int brackets = 0, braces = 0;
  int gameend = FALSE;
  move_s moves[MOVE_BUFF];
  int match, num_moves, i;
  int limit = 0;
  
  /* reset board */
  init_game();
  initialize_hash();
  
  putchar('.');
  
  while (!gameend)
    {
      ch = getc(pgnbook);
      
      if (ch == EOF)
	return;
      
      if (ch == ' ' || ch == '\n')
	{
	  /* just skip and go on */
	}
      else if (ch == '{')
	{
	  brackets++;
	  /* we want to skip everything till we get brackets 
	   * and braces back to 0 */
	  
	  while (brackets > 0 || braces > 0)
	    {
	      xch = getc(pgnbook);
	      if (xch == '}')
		brackets--;
	      else if (xch == '{')
		brackets++;
	      else if (xch == '[')
		braces++;
	      else if (xch == ']')
		braces--;
	      else if (xch == EOF)
		break;
	    }
	}
      else if (ch == '[')
	{
	  braces++;
	  while (brackets > 0 || braces > 0)
	    {
	      xch = getc(pgnbook);
	      if (xch == '}')
		brackets--;
	      else if (xch == '{')
		brackets++;
	      else if (xch == '[')
		braces++;
	      else if (xch == ']')
		braces--;
	      else if (xch == EOF)
		break;
				}
	}
      else if (ch == '*')
	{
	  /* result string: unfinished game */
	  /* seek next header */
	  while (((ch = getc(pgnbook)) != '[') && !feof(pgnbook));
	  ungetc(ch, pgnbook);
	  gameend = TRUE;
	}
      else if (isdigit(ch))
	{
	  xch = getc(pgnbook);
	  
	  if (xch == EOF) 
	    {
	      return;
	    }
	  /* either a move number or a result string */
	  else if (isdigit(xch))   /* 2 digits...must be move number */
	    {
	      while(((ch = getc(pgnbook)) != '.') && !feof(pgnbook));
	    }
	  else if (xch != '.')
	    {
	      /* not a move numer, must be result */
	      /* seek to next header */
	      while (((ch = getc(pgnbook)) != '[') && !feof(pgnbook));
	      ungetc(ch, pgnbook);
	      
	      gameend = TRUE;
	    }
	}
      else if (isalpha(ch))
	{
	  /* parse one move */
	  ms = 0;
	  movebuff[ms++] = ch;
	  
	  while(movebuff[ms-1] != ' ' && movebuff[ms-1] != '\n')
	    {
	      movebuff[ms++] = getc(pgnbook);
	    }
	  movebuff[--ms] = '\0'; /* scratch last bogus char */
	  
	  /* movebuff now contains -hopefully- the move in SAN */
	  //	printf("Read move: -%s- ", &movebuff);
	  
	  /* now, generate all moves from the current pos and try
	   * to get a match */
	  match = FALSE;
	  num_moves = 0;
	  gen (&moves[0]); 
	  num_moves = numb_moves;
	  for (i = 0; i < num_moves; i++)
	    {
	      comp_to_san(moves[i], sjmove);
	      if (!strcmp(movebuff, sjmove))
		{
		  /* moves matched !*/
		  make(&moves[0], i);
		  match = TRUE;
		  if (check_legal(&moves[0], i))
		    {
		      break;
		    }
		  else
		    printf("Illegal move from PGN!\n");
		}
	    }
	  
	  limit++;
	  
	  if (match == FALSE || limit > 40)
	    {
	      /* skip junk game */
	      while (((ch = getc(pgnbook)) != '[') && !feof(pgnbook));
	      ungetc(ch, pgnbook);
	      gameend = TRUE;
	    }
	  else
	    {
	      add_current(binbook, pgn_header);
	    }
	}
    }
}

void weed_book(GDBM_FILE binbook)
{
  datum data;
  datum index;
  datum nextkey;
  posinfo_t *ps;
  int weeds;
  int positions;
  
  do
    {
      weeds = 0;
      positions = 0;
      
      index = gdbm_firstkey(binbook);
      
      while (index.dptr) 
	{
	  positions++;
	  
	  nextkey = gdbm_nextkey (binbook, index);
	  
	  data = gdbm_fetch(binbook, index);
	  ps = data.dptr;   
	  
	  if ((ps->played) < PLAYTHRESHOLD) 
	    {
	      gdbm_delete(binbook, index);
	      free(index.dptr);
	      weeds++;
	    }
	  
	  free(data.dptr);
	  index = nextkey;
       	}
      
      printf("Weeded %d moves.\n", weeds);
    } 
  while (weeds > 0);
  
  printf("%d unique positions.\n", positions);

  printf("Reorganizing BinBook.\n");
  gdbm_reorganize(binbook);
  
  printf("Done.\n");	
}

void build_book (void)
{
  FILE *pgnbook;
  GDBM_FILE binbook;
  pgn_header_t pgn_header;
  char bookname[FILENAME_MAX], kks[STR_BUFF];
  
  printf("\nName of PGN book: ");
  rinput(bookname, STR_BUFF, stdin);
  
  pgnbook = fopen(bookname, "r");
  
  if (pgnbook == NULL)
    {
      printf("PGN book not found!\n");
      exit(EXIT_FAILURE);
    }
  
  if (Variant == Normal)
    binbook = gdbm_open("nbook.bin", 16384, GDBM_NEWDB | GDBM_FAST, 00664, NULL);
  else if (Variant == Suicide)
    binbook = gdbm_open("sbook.bin", 16384, GDBM_NEWDB | GDBM_FAST, 00664, NULL);
  else
    binbook = gdbm_open("zbook.bin", 16384, GDBM_NEWDB | GDBM_FAST, 00664, NULL);
    
  
  if (binbook == NULL)
    {
      printf("Error opening binbook.\n");
      exit(EXIT_FAILURE);
    }
  
  printf("\nSize of KeyCache (bytes): ");
  rinput(kks, STR_BUFF, stdin);
  
  kksize = atol(kks);
  
  keycache = (unsigned char *) calloc(kksize, sizeof(unsigned char));
  
  if (keycache == NULL)
    {
      printf("Not enough RAM!\n");
      exit(EXIT_FAILURE);
    }
  
  while (!feof(pgnbook))
    {
      get_header(pgnbook, &pgn_header);
      replay_game(pgnbook, binbook, pgn_header);
    };
  
  free(keycache);
  
  printf("\nWeeding book moves.\n");
  weed_book(binbook);
  
  fclose(pgnbook);
  gdbm_close(binbook);
}


move_s choose_binary_book_move (void) 
{
  GDBM_FILE binbook;
  hashkey_t key;
  posinfo_t *ps;
  datum index;
  datum data;
  move_s moves[MOVE_BUFF], bestmove;
  move_s bookmoves[MOVE_BUFF];
  int num_bookmoves;
  int raw;
  int num_moves, i, ep;
  char output[6];
  unsigned long scores[MOVE_BUFF], best_score = 0;
  
  srand(time(0));
  
  if (Variant == Normal)
    binbook = gdbm_open("nbook.bin", 16384, GDBM_READER, 0, NULL);
  else if (Variant == Suicide)
    binbook = gdbm_open("sbook.bin", 16384, GDBM_READER, 0, NULL);
  else 
    binbook = gdbm_open("zbook.bin", 16384, GDBM_READER, 0, NULL);
    
  
  if (binbook == NULL)
    {
      printf("No BinBook found.\n");
      return dummy;
    }
  
  num_moves = 0;
  raw = 0;
  num_bookmoves = 0;
  
  gen(&moves[0]);
  num_moves = numb_moves;	
  
  for (i = 0; i < num_moves; i++)
    {
      make(&moves[0], i);
      
      if (check_legal(&moves[0], i))
	{

	  key.hashkey = (hash ^ ToMove);
	  index.dptr = &key;
	  index.dsize = sizeof(key);
	  
	  data = gdbm_fetch(binbook, index);
	  
	  if (data.dptr != NULL)
	    {
	      ps = data.dptr;
	      
	      raw++;
			
	      comp_to_coord(moves[i], output);
	      
	      printf("Move %s: %d times played\n", output,
		     ps->played);
	      
	      if ((ps->played) >=  PLAYTHRESHOLD)
		{
		  scores[num_bookmoves] = ps->played;
		  bookmoves[num_bookmoves] = moves[i];
		  num_bookmoves++;
		}
	      
	      free(data.dptr);
	    }
	}
      
      unmake(&moves[0], i);
    }
  
  gdbm_close(binbook);
  
  printf("Book moves: raw: %d cut : %d\n", raw, num_bookmoves);
  
  if (!num_bookmoves) 
    return dummy;
  
  lastbookpos = hash;
  lastbooktomove = ToMove;

  /* find the top frequency: */
    for (i = 0; i < num_bookmoves; i++) {
      if (scores[i] > best_score) {
        best_score = scores[i];
      }
    }
    
    /* add some randomness to each frequency: */
    for (i = 0; i < num_bookmoves; i++) {
      scores[i] += (int) ((float)(((float)(rand())/RAND_MAX)) 
			  * ((float)best_score*1.25));
    }

    /* now pick our best move: */
    best_score = 0;
    for (i = 0; i < num_bookmoves; i++) {
      if (scores[i] > best_score) {
	best_score = scores[i];
	bestmove = bookmoves[i];
      }
    }
    
    return bestmove;   
}


void book_learning(int result)
{
  GDBM_FILE binbook;
  hashkey_t key;
  posinfo_t *ps;
  datum index;
  datum data;
  int playinc;

  if (lastbookpos == 0) return;
  
  if (Variant == Normal)
    binbook = gdbm_open("nbook.bin", 16384, GDBM_READER, 0, NULL);
  else if (Variant == Suicide)
    binbook = gdbm_open("sbook.bin", 16384, GDBM_READER, 0, NULL);
  else 
    binbook = gdbm_open("zbook.bin", 16384, GDBM_READER, 0, NULL);
    
   
  if (binbook == NULL)
    {
      printf("No BinBook found, not learning.\n");
      return;
    }  

  key.hashkey = (lastbookpos ^ lastbooktomove);
  index.dptr = &key;
  index.dsize = sizeof(key);
  
  data = gdbm_fetch(binbook, index);
  
  if (data.dptr != NULL)
    {
      ps = data.dptr;

      playinc = 0;
      
      if (result == WIN)
	{
	  if (my_rating <= opp_rating)
	    playinc = 2;
	  else
	    playinc = 1;
	}
      else if (result == LOSS)
	{
	  if (my_rating >= opp_rating)
	    playinc = -2;
	  else
	    playinc = -1;
	}
      else
	{
	  if (my_rating >= opp_rating)
	    playinc = -1;
	  else
	    playinc = 1;
	}

      if ((ps->played + playinc) < 0)
	playinc = -(ps->played);

      printf("Learning opening %X, old %d, new %d\n", 
	     lastbookpos, ps->played, (ps->played)+playinc);

      ps->played += playinc;

      gdbm_store(binbook, index, data, GDBM_REPLACE);      
    
      free(data.dptr);
    }
  else
    {
      printf("Book position disappeared during game?!?\n");
    }

  return;
};
