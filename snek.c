#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#define SNEK_HEAD 'S'
#define SNEK_TAIL 's'
#define SNAK_CHAR '@'
#define WALL_CHAR '#'
#define FLOOR_CHAR ' '

#define FRAME_RATE 6

#define LEVEL_W 30
#define LEVEL_H 15

/* structs */

struct Player {
	int locy;
	int locx;
	int diry;
	int dirx;
	bool alive;
	int* taily;
	int* tailx;
	int score;
	bool snak_exists;
} player;

struct Snak {
	int locy;
	int locx;
	bool exists; 
} snak;

/* start, run, and end program */

void start_ncurses() {
  initscr();
  cbreak();
  noecho();
	nodelay(stdscr, true);
	curs_set(false);
}

void stop_ncurses() {
	curs_set(true);
	nodelay(stdscr, false);
  echo();
  nocbreak();
  endwin();
}

/* helper functions */

bool is_some_char(int y, int x, char c) {
	if (mvinch(y, x) == c) {
		return true;
	}
	return false;
}

int* push(int* arr, int size, int value) {
	/* puts value in index 0 of arr, pushing all other values right */
	for (int i = size; i >= 0; i--) arr[i+1] = arr[i];
	arr[0] = value;
	return arr;
}

/* player related functions */

void init_player(struct Player* pl) {
	pl -> locy = 1;
	pl -> locx = 1;
	pl -> diry = 0;
	pl -> dirx = 0;
	pl -> alive = true;
	pl -> taily = (int*)calloc(LEVEL_H*LEVEL_W, sizeof(int));
	pl -> tailx = (int*)calloc(LEVEL_H*LEVEL_W, sizeof(int));
	pl -> score = 0;
	pl -> snak_exists = false;
}

bool is_walkable(int y, int x) {
	if (is_some_char(y, x, WALL_CHAR) || is_some_char(y, x, SNEK_TAIL)) {
		return false;
	} else {
		return true;
	}
}

void move_player(struct Player* pl) {
	/* Moves head, and pushes last location onto tail */
	int to_y = pl -> locy + pl -> diry;
	int to_x = pl -> locx + pl -> dirx;
	if (is_walkable(to_y, to_x)) {
		pl -> taily = push(pl -> taily, pl -> score + 1, pl -> locy);
		pl -> tailx = push(pl -> tailx, pl -> score + 1, pl -> locx);
		pl -> locy = to_y;
		pl -> locx = to_x;
	} else {
		pl -> alive = false;
	}
}

void draw_player(struct Player* pl) {
	for (int i = 0; i < pl -> score + 1; i++) {
		if (pl -> taily[i] != 0 && pl -> tailx[i] != 0)
			mvaddch(pl -> taily[i], pl -> tailx[i], SNEK_TAIL);
	}
	mvaddch(pl -> locy, pl -> locx, SNEK_HEAD);
}

void grow_player(struct Player* pl) {
	pl -> score++;
}

/* snak related functions */

void init_snak(struct Snak* sn) {
	/* Picks a random location to place a snak */
	int y;
	int x;
	do {
		y = (rand() % LEVEL_H) + 1;
		x = (rand() % LEVEL_W) + 1;
	} while (!is_some_char(y, x, FLOOR_CHAR));
	mvaddch(y, x, SNAK_CHAR);
	sn -> locy = y;
	sn -> locx = x;
	sn -> exists = true;
}

void draw_snak(struct Snak* sn) {
	mvaddch(sn -> locy, sn -> locx, SNAK_CHAR);
}

// TODO: Add a collect_snak function that increases the player's score
// and size, as well as setting sn -> exists = false; 
void collect_snak(struct Player* pl, struct Snak* sn) {
	if (pl -> locy == sn -> locy && pl -> locx == sn -> locx) {
		sn -> exists = false;
		grow_player(pl);
	}
}

/* draw the scoreboard */
void draw_score(struct Player* pl) {
	mvprintw(LEVEL_H + 2, 1, "Score: %d", pl -> score);
}

/* draw the level */

void draw_level() {
	/* draws the walls and floor */
	for (int y = 0; y <= LEVEL_H; y++) {
		mvhline(y, 1, FLOOR_CHAR, LEVEL_W);
  }
	mvhline(0, 0, WALL_CHAR, LEVEL_W+2);
	mvhline(LEVEL_H+1, 0, WALL_CHAR, LEVEL_W+2);
	mvvline(0, 0, WALL_CHAR, LEVEL_H+2);
	mvvline(0, LEVEL_W+1, WALL_CHAR, LEVEL_H+2);
}

/* where all the stuff happens */

void game_loop() {
	struct Player* pl = &player;
	struct Snak* sn = &snak;
	init_player(pl);
	while (true) {
		/* check player input */
		char ch = getch();
		if (ch == 'q') break;
		switch(ch) {
			case 'w':
				pl -> diry = -1;
				pl -> dirx = 0;
				break;
			case 's':
				pl -> diry = 1;
				pl -> dirx = 0;
				break;
			case 'a':
				pl -> diry = 0;
				pl -> dirx = -1;
				break;
			case 'd':
				pl -> diry = 0;
				pl -> dirx = 1;
				break;
		}
		move_player(pl);
		if (!(pl -> alive)) break;
		if (!(sn -> exists)) init_snak(sn);
		collect_snak(pl, sn);

		draw_level();
		draw_snak(sn);
		draw_player(pl);
		draw_score(pl);
		
		refresh();
		napms(1000/FRAME_RATE);
	}
	/* free player tail memory */
	free(pl -> taily);
	free(pl -> tailx);
}

int main(void) {
  start_ncurses();
	game_loop();
  stop_ncurses();
}
