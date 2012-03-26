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
                                                          
    File: sjeng.c
    Purpose: main program, xboard/user interface                  

*/

#include "sjeng.h"
#include "protos.h"
#include "extvars.h"
#include "config.h"

char divider[50] = "-------------------------------------------------";
move_s dummy = {0,0,0,0,0};

int board[144], moved[144], ep_square, white_to_move, comp_color, wking_loc,
  bking_loc, white_castled, black_castled, result, ply, pv_length[PV_BUFF],
  pieces[62], squares[144], num_pieces, i_depth;

long int nodes, raw_nodes, qnodes, piece_count, killer_scores[PV_BUFF],
  killer_scores2[PV_BUFF], killer_scores3[PV_BUFF], moves_to_tc, min_per_game,
  inc, time_left, opp_time, time_cushion, time_for_move, cur_score;

unsigned long history_h[144][144];

bool xb_mode, captures, searching_pv, post, time_exit, time_failure;

int phase;
int root_to_move;


move_s pv[PV_BUFF][PV_BUFF], killer1[PV_BUFF], killer2[PV_BUFF],
 killer3[PV_BUFF];

rtime_t start_time;

int is_promoted[62];

int NTries, NCuts, TExt;
unsigned long DeltaTries, DeltaCuts;
unsigned long PVS, FULL, PVSF;
unsigned long ext_check;

bool is_pondering, allow_pondering, is_analyzing;

int Variant;

char my_partner[STR_BUFF];
bool have_partner;
bool must_sit;
bool go_fast;

long fixed_time;

FILE *lrn_standard;
FILE *lrn_zh;
FILE *lrn_suicide;

int main (int argc, char *argv[]) {

  char input[STR_BUFF], *p, output[STR_BUFF];
  char readbuff[STR_BUFF];
  move_s move, comp_move;
  int depth = 4;
  bool force_mode, show_board;
  double nps, elapsed;
  clock_t cpu_start, cpu_end;
  move_s game_history[600];
  int ep_squares[600];
  int move_number;
  int is_edit_mode, edit_color;

  initialize_zobrist();
  initialize_eval();
 
  Variant = Normal;
  //Variant = Crazyhouse;

  memcpy(material, std_material, sizeof(std_material));
  //memcpy(material, zh_material, sizeof(zh_material));

  if (!init_book())
    printf("No opening book found.\n");

  if ((lrn_standard = fopen ("standard.lrn", "rb+")) == NULL)
    {
      printf("No standard learn file.\n");
      
      if ((lrn_standard = fopen ("standard.lrn", "wb+")) == NULL)
	{
	  printf("Error creating standard learn file.\n");
	}
      else
	{
	  fclose(lrn_standard);
	  lrn_standard = fopen ("standard.lrn", "rb+");
	}
    }
  if ((lrn_zh = fopen ("bug.lrn", "rb+")) == NULL)
    {
      printf("No crazyhouse learn file.\n");

      if ((lrn_zh = fopen ("bug.lrn", "wb+")) == NULL)
	{
	  printf("Error creating crazyhouse learn file.\n");
	}
      else
	{
	  fclose(lrn_zh);
	  lrn_zh = fopen ("bug.lrn", "rb+");
	}
    }
  if ((lrn_suicide = fopen ("suicide.lrn", "rb+")) == NULL)
    {
      printf("No suicide learn file.\n");

      if ((lrn_suicide = fopen ("suicide.lrn", "wb+")) == NULL)
	{
	  printf("Error creating suicide learn file.\n");
	}
      else
	{
	  fclose(lrn_suicide);
	  lrn_suicide = fopen ("suicide.lrn", "rb+");
	}
    }

  start_up ();
  init_game ();

  initialize_hash();
  clear_tt();
  ECacheProbes = 0;
  ECacheHits = 0;
  TTProbes = 0;
  TTStores = 0;
  TTHits = 0;

  xb_mode = FALSE;
  force_mode = FALSE;
  comp_color = 0;
  show_board = TRUE;
  is_pondering = FALSE;
  allow_pondering = TRUE;
  is_analyzing = FALSE;
  is_edit_mode = FALSE;
  have_partner = FALSE;
  must_sit = FALSE;
  go_fast = FALSE;
  fixed_time = FALSE;
  phase = Opening;
  root_to_move = WHITE;
  kibitzed = FALSE;

  move_number = 0;
  memset(game_history, 0, sizeof(game_history));
  
  setbuf (stdout, NULL);
  setbuf (stdin, NULL);

  /* keep looping for input, and responding to it: */
  while (TRUE) {

    /* case where it's the computer's turn to move: */
    if (!is_edit_mode && comp_color == white_to_move && !force_mode && !must_sit) {

      /* whatever happens, never allow pondering in normal search */
      is_pondering = FALSE;
  
      cpu_start = clock ();
      comp_move = think ();
      cpu_end = clock();

      /* must_sit can be changed by search */
      if (!must_sit)
	{
	  /* check for a game end: */
	  if ((comp_color == 1 && result != white_is_mated) ||
	      (comp_color == 0 && result != black_is_mated)) {
	    
	    comp_to_coord (comp_move, output);
	    
	    game_history[move_number] = comp_move;
	    ep_squares[move_number++] = ep_square;
	    make (&comp_move, 0);
	    
	    root_to_move ^= 1;

	    reset_piece_square ();
	    
	    if (book_ply < 40) {
	      if (!book_ply) {
		strcpy(opening_history, output);
	      }
	      else {
		strcat(opening_history, output);
	      }
	    }
	    
	    book_ply++;
	    
	    printf ("\nNodes: %ld (%0.2f%% qnodes)\n", nodes,
		    (float) ((float) qnodes / (float) nodes * 100.0));
	    
	    elapsed = (cpu_end-cpu_start)/(double) CLOCKS_PER_SEC;
	    nps = (float) nodes/(float) elapsed;
	    
	    if (!elapsed)
	      printf ("NPS: N/A\n");
	    else
	      printf ("NPS: %ld\n", (long int) nps);
	    
	    printf("ECacheProbes : %ld   ECacheHits : %ld   HitRate : %f%%\n", 
		   ECacheProbes, ECacheHits, 
		   ((float)ECacheHits/((float)ECacheProbes+1)) * 100);
	    
	    printf("TTStores : %ld TTProbes : %ld   TTHits : %ld   HitRate : %f%%\n", 
		   TTStores, TTProbes, TTHits, 
		   ((float)TTHits/((float)TTProbes+1)) * 100);
	    
	    printf("NTries : %d  NCuts : %d  CutRate : %f%%  TExt: %d\n", 
		   NTries, NCuts, (((float)NCuts*100)/((float)NTries+1)), TExt);
	    
	    printf("DeltaTries : %d  DeltaCuts : %d  CutRate : %f%%\n",
		   DeltaTries, DeltaCuts, 
		   ((float)(DeltaCuts*100)/(((float)DeltaTries+1))));
	    printf("Check extensions: %ld  Razor drops : %ld  Razor Material : %ld\n", ext_check, razor_drop, razor_material);
	    
	    printf("Move ordering : %f%%\n", (((float)FHF*100)/(float)FH+1));
	    
	    printf("Material score: %d   Eval : %d\n", Material, eval());
	    
	    printf("Hash : %X  HoldHash : %X\n", hash, hold_hash);

	    /*printf("Average moves: %f\n", (float)total_moves/(float)total_movegens);*/

	    
	    /* check to see if we mate our opponent with our current move: */
	    if (!result) {
	      if (xb_mode) {

		/* safety in place here */
		if (comp_move.from != dummy.from || comp_move.target != dummy.target)
		    printf ("move %s\n", output);

		if (Variant == Bughouse)
		  {
		    CheckBadFlow(FALSE);
		  }	
	      }
	      else {
		printf ("\n%s\n", output);
      	      }
	    }
	    else {
	      if (xb_mode) {
		if (comp_move.from != dummy.from || comp_move.target != dummy.target)
		    printf ("move %s\n", output);
	      }
	      else {
		printf ("\n%s\n", output);
	      }
	      if (result == white_is_mated) {
		printf ("0-1 {Black Mates}\n");
	      }
	      else if (result == black_is_mated) {
		printf ("1-0 {White Mates}\n");
	      }
	      else {
		printf ("1/2-1/2 {Draw}\n");
	      }
	    }
	  }
	  /* we have been mated or stalemated: */
	  else {
	    if (result == white_is_mated) {
	      printf ("0-1 {Black Mates}\n");
	    }
	    else if (result == black_is_mated) {
	      printf ("1-0 {White Mates}\n");
	    }
	    else {
	      printf ("1/2-1/2 {Draw}\n");
	    }
	  }
	}
    }

    /* get our input: */
    if (!xb_mode) {
      if (show_board) {
	printf ("\n");
	display_board (stdout, 1-comp_color);
      }
      printf ("Sjeng: ");
      rinput (input, STR_BUFF, stdin);
    }
    else {
      /* start pondering */

      if ((must_sit || (allow_pondering && !is_edit_mode && !force_mode) || is_analyzing ))
	{
	  is_pondering = TRUE;
	  
	  think();
	  
	  is_pondering = FALSE;
	}

      rinput (input, STR_BUFF, stdin);
    }

    /* check to see if we have a move.  If it's legal, play it. */
    if (!is_edit_mode && is_move (&input[0])) {
      if (verify_coord (input, &move)) {
	game_history[move_number] = move;
	ep_squares[move_number++] = ep_square;
	make (&move, 0);
	reset_piece_square ();
	
	root_to_move ^= 1;
	
	if (book_ply < 40) {
	  if (!book_ply) {
	    strcpy(opening_history, input);
	  }
	  else {
	    strcat(opening_history, input);
	  }
        }
	
	book_ply++;
	
	if (show_board) {
	  printf ("\n");
	  display_board (stdout, 1-comp_color);
	}
      }
      else {
	printf ("Illegal move: %s\n", input);
	}
    }
    else {

      /* make everything lower case for convenience: */
      for (p = input; *p; p++) *p = tolower (*p);

      /* command parsing: */
      if (!strcmp (input, "quit")) {
	exit (EXIT_SUCCESS);
      }
      else if (!strcmp (input, "exit"))
	{
	  if (is_analyzing)
	    {
	      is_analyzing = FALSE;
	      is_pondering = FALSE;
	      time_for_move = 0;
	    }
	  else
	    {
	      fclose(lrn_standard);
	      fclose(lrn_zh);
	      fclose(lrn_suicide);
	      exit (EXIT_SUCCESS);
	    }
	}
      else if (!strcmp (input, "diagram") || !strcmp (input, "d")) {
	toggle_bool (&show_board);
      }
      else if (!strncmp (input, "perft", 5)) {
	sscanf (input+6, "%d", &depth);
	raw_nodes = 0;
	perft (depth);
	printf ("Raw nodes for depth %d: %ld\n", depth, raw_nodes);
      }
      else if (!strcmp (input, "new")) {

	if (xb_mode)
	  {
	    printf("tellics set 1 Sjeng " VERSION " (2000-12-4)\n");
	  }

	if (!is_analyzing)
	{	
	  memcpy(material, std_material, sizeof(std_material));
	  Variant = Normal;

	  //memcpy(material, zh_material, sizeof(zh_material));
	  //Variant = Crazyhouse;
	    
	  init_game ();
	  initialize_hash();
	  clear_tt();
	  init_book();
	  reset_ecache();	
  
	  force_mode = FALSE;
	  must_sit = FALSE;
	  go_fast = FALSE;
	  piecedead = FALSE;
	  partnerdead = FALSE;
	  kibitzed = FALSE;

	  root_to_move = WHITE;
  
	  comp_color = 0;
	  move_number = 0;

	  CheckBadFlow(TRUE);
	  ResetHandValue();
	}
	else
	{
	  init_game ();
	  move_number = 0;
	}
	
      }
      else if (!strcmp (input, "xboard")) {
	xb_mode = TRUE;
	toggle_bool (&show_board);
	signal (SIGINT, SIG_IGN);
	printf ("\n");
	
	/* Reset f5 in case we left with partner */
	printf("tellics set f5 crazyhouse\n");
	
	BegForPartner();
      }
      else if (!strcmp (input, "nodes")) {
	printf ("Number of nodes: %ld (%0.2f%% qnodes)\n", nodes,
		(float) ((float) qnodes / (float) nodes * 100.0));
      }
      else if (!strcmp (input, "nps")) {
	elapsed = (cpu_end-cpu_start)/(double) CLOCKS_PER_SEC;
	nps = (float) nodes/(float) elapsed;
	if (!elapsed)
	  printf ("NPS: N/A\n");
	else
	  printf ("NPS: %ld\n", (long int) nps);
      }
      else if (!strcmp (input, "post")) {
	toggle_bool (&post);
	if (xb_mode)
	  post = TRUE;
      }
      else if (!strcmp (input, "nopost")) {
	post = FALSE;
      }
      else if (!strcmp (input, "random")) {
	continue;
      }
      else if (!strcmp (input, "hard")) {

	allow_pondering = TRUE;

	continue;
      }
      else if (!strcmp (input, "easy")) {

	allow_pondering = FALSE;

	continue;
      }
      else if (!strcmp (input, "?")) {
	continue;
      }
      else if (!strcmp (input, "white")) {
	//if (is_edit_mode)
	//{
	    white_to_move = 1;
	    root_to_move = WHITE;
	//};
	comp_color = 0;
      }
      else if (!strcmp (input, "black")) {
	//if (is_edit_mode)
	//{
	    white_to_move = 0;
	    root_to_move = BLACK;
	//};
	comp_color = 1;
      }
      else if (!strcmp (input, "force")) {
	force_mode = TRUE;
      }
      else if (!strcmp (input, "go")) {
	comp_color = white_to_move;
	force_mode = FALSE;
      }
      else if (!strncmp (input, "time", 4)) {
	sscanf (input+5, "%ld", &time_left);
      }
      else if (!strncmp (input, "otim", 4)) {
	sscanf (input+5, "%ld", &opp_time);
      }
      else if (!strncmp (input, "level", 5)) {
	/* extract the time controls: */
	sscanf (input+6, "%ld %ld %ld", &moves_to_tc, &min_per_game, &inc);
	time_left = min_per_game*6000;
	opp_time = time_left;
	fixed_time = FALSE;
      }
      else if (!strncmp (input, "result", 6)) {

      }
      else if (!strncmp (input, "holding", 7)) {
	ProcessHoldings(input);     
      }
      else if (!strncmp (input, "variant", 7)) {
	if (strstr(input, "normal"))
	  {
	    Variant = Normal;
	    memcpy(material, std_material, sizeof(std_material));
	    init_book();
	  }
	else if (strstr(input, "crazyhouse"))
	  {
	    Variant = Crazyhouse;
	    memcpy(material, zh_material, sizeof(zh_material));
	    init_book();
	  }
	else if (strstr(input, "bughouse"))
	  {
	    Variant = Bughouse;
	    memcpy(material, zh_material, sizeof(zh_material));
	    init_book();
	  }
	else if (strstr(input, "suicide"))
	  {
	    Variant = Suicide;
	    memcpy(material, suicide_material, sizeof(suicide_material));
	    init_book();
	  }
	else if (strstr(input, "giveaway"))
	  {
	    Variant = Suicide;
	    memcpy(material, suicide_material, sizeof(suicide_material));
	    init_book();
	  }
	
	initialize_hash();
	clear_tt();
	reset_ecache();

      }
      else if (!strncmp (input, "analyze", 7)) {
	is_analyzing = TRUE;
	is_pondering = TRUE;
	think();
      }
      else if (!strncmp (input, "undo", 4)) {
	    printf("Move number : %d\n", move_number);
	if (move_number > 0)
	  {
	    printf("UNMAKING\n");
	    unmake(&game_history[--move_number], 0);
	    ep_square = ep_squares[move_number];
	    reset_piece_square();
	    root_to_move ^= 1;
	  }
      }
      else if (!strncmp (input, "remove", 5)) {
	if (move_number > 1)
	  {
	    unmake(&game_history[--move_number], 0);
	    ep_square = ep_squares[move_number];
	    reset_piece_square();
	    unmake(&game_history[--move_number], 0);
	    ep_square = ep_squares[move_number];
	    reset_piece_square();
	  }
      }
      else if (!strncmp (input, "edit", 4)) {
	is_edit_mode = TRUE;
	edit_color = WHITE;
      }
      else if (!strncmp (input, ".", 1) && is_edit_mode) {
	is_edit_mode = FALSE;
	if (wking_loc == 30) white_castled = no_castle;
	if (bking_loc == 114) black_castled = no_castle;
	book_ply = 50;
	ep_square = 0;
	move_number = 0;
	memset(opening_history, 0, sizeof(opening_history));
	clear_tt();
	initialize_hash();
	reset_piece_square();
      }
      else if (is_edit_mode && !strncmp (input, "c", 1)) {
	if (edit_color == WHITE) edit_color = BLACK; else edit_color = WHITE;
	}
      else if (is_edit_mode && !strncmp (input, "#", 1)) {
	reset_board();
	move_number = 0;
      }
      else if (is_edit_mode 
	       && isalpha(input[0]) 
	       && isalpha(input[1]) 
	       && isdigit(input[2])) {
	PutPiece(edit_color, input[0], input[1], input[2]);
      }
      else if (!strncmp (input, "partner", 7)) {
	HandlePartner(input);
	}
      else if (!strncmp (input, "ptell", 5)) {
	HandlePtell(input);
      }
      else if (!strncmp (input, "test", 4)) {
	run_epd_testsuite();
      }
      else if (!strncmp (input, "st", 2)) {
	sscanf(input+3, "%d", &fixed_time); 
      }
      else if (!strncmp (input, "book", 4)) {
	build_book();
      }
      else if (!strncmp (input, "speed",  5)) {
	speed_test();
      }
      else if (!strncmp (input, "prove", 5)) {
	printf("\nMax time to search (s): ");
	start_time = rtime();
	rinput(readbuff, STR_BUFF, stdin);
	pn_time = atol(readbuff) * 100;
	printf("\n");
	proofnumbersearch();      
       }
      else if (!strncmp (input, "warranty", 8)) {
	  printf("\n  BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
		 "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
		 "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
		 "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
		 "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
		 "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
		 "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
		 "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
		 "REPAIR OR CORRECTION.\n"
		 "\n");
	  printf("  IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
		 "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
		 "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
		 "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
		 "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
		 "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
		 "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
		 "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
		 "POSSIBILITY OF SUCH DAMAGES.\n\n");

	}
      else if (!strncmp (input, "distribution", 12)) {
	printf("\n  You may copy and distribute verbatim copies of the Program's\n"
	       "source code as you receive it, in any medium, provided that you\n"
	       "conspicuously and appropriately publish on each copy an appropriate\n"
	       "copyright notice and disclaimer of warranty; keep intact all the\n"
	       "notices that refer to this License and to the absence of any warranty;\n"
	       "and give any other recipients of the Program a copy of this License\n"
	       "along with the Program.\n"
	       "\n"
	       "You may charge a fee for the physical act of transferring a copy, and\n"
	       "you may at your option offer warranty protection in exchange for a fee.\n\n");

	}
      else if (!strcmp (input, "help")) {
	printf ("\n%s\n\n", divider);
	printf ("diagram/d:    toggle diagram display\n");
	printf ("exit/quit:    terminate Sjeng\n");
	printf ("go:           make Sjeng play the side to move\n");
	printf ("new:          start a new game\n");
	printf ("level <x>:    the xboard style command to set time\n");
	printf ("  <x> should be in the form: <a> <b> <c> where:\n");
	printf ("  a -> moves to TC (0 if using an ICS style TC)\n");
	printf ("  b -> minutes per game\n");
	printf ("  c -> increment in seconds\n");
	printf ("nodes:        outputs the number of nodes searched\n");
	printf ("nps:          outputs Sjeng's NPS in search\n");
	printf ("perft <x>:    compute raw nodes to depth x\n");
	printf ("post:         toggles thinking output\n");
	printf ("xboard:       put Sjeng into xboard mode\n");
	printf ("test:         run an EPD testsuite\n");
	printf ("speed:        test movegen and evaluation speed\n");
	printf ("warranty:     show warranty details\n");
	printf ("distribution: show distribution details\n");
	printf( "proof:        try to prove or disprove the current pos\n");
	printf ("\n%s\n\n", divider);
      }
      else if (!xb_mode) {
	printf ("Illegal move: %s\n", input);
      }

    }

  }

  return 0;

}
