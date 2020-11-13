#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#define SNEK_HEAD 'S'
#define SNEK_TAIL 's'
#define SNAK_CHAR '@'
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
};

struct Snak {
	int locy;
	int locx;
	bool exists; 
};

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

bool is_some_char(WINDOW* win, int y, int x, char c) {
	if (mvwinch(win, y, x) == c) {
		return true;
	}
	return false;
}

int* push(int* arr, int size, int value) {
	/* puts value in index 0 of arr, pushing all other values right */
	for (int i = size-2; i >= 0; i--) *(arr+i+1) = *(arr+i);
	arr[0] = value;
	return arr;
}

/* handling ncurses screens */

WINDOW* init_win(int h, int w, int y, int x) {
	WINDOW* win = newwin(h, w, y, x);
	wrefresh(win);
	return win;
}

WINDOW* kill_win(WINDOW* win) {
	wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(win);
	delwin(win);
}

/* player related functions */

struct Player* init_player() {
	struct Player* pl = (struct Player*)malloc(sizeof(struct Player));
	pl -> locy = 1;
	pl -> locx = 1;
	pl -> diry = 0;
	pl -> dirx = 0;
	pl -> alive = true;
	pl -> taily = (int*)calloc(2, sizeof(int));
	pl -> tailx = (int*)calloc(2, sizeof(int));
	pl -> score = 0;
	pl -> snak_exists = false;
	return pl;
}

bool is_walkable(WINDOW* win, int y, int x) {
	if (is_some_char(win, y, x, SNEK_TAIL) 
		|| is_some_char(win, y, x, mvwinch(win, 0, 1))
		|| is_some_char(win, y, x, mvwinch(win, 1, 0))) {
		return false;
	} else {
		return true;
	}
}

void move_player(WINDOW* win, struct Player* pl) {
	/* Moves head, and pushes last location onto tail */
	int to_y = pl -> locy + pl -> diry;
	int to_x = pl -> locx + pl -> dirx;
	if (is_walkable(win, to_y, to_x)) {
		pl -> taily = push(pl -> taily, pl -> score + 1, pl -> locy);
		pl -> tailx = push(pl -> tailx, pl -> score + 1, pl -> locx);
		pl -> locy = to_y;
		pl -> locx = to_x;
	} else {
		pl -> alive = false;
	}
}

void draw_player(WINDOW* win, struct Player* pl) {
	for (int i = 0; i < pl -> score + 1; i++) {
		if (pl -> taily[i] != 0 && pl -> tailx[i] != 0)
			mvwaddch(win, pl -> taily[i], pl -> tailx[i], SNEK_TAIL);
	}
	mvwaddch(win, pl -> locy, pl -> locx, SNEK_HEAD);
}

void grow_player(struct Player* pl) {
	pl -> score++;
	pl -> taily  = realloc(pl -> taily, (1 + pl -> score)*sizeof(int));
	pl -> tailx  = realloc(pl -> tailx, (1 + pl -> score)*sizeof(int));
}

/* snak related functions */

struct Snak* init_snak(WINDOW* win) {
	/* Picks a random location to place a snak */
	struct Snak* sn = (struct Snak*)malloc(sizeof(struct Snak));
	int y;
	int x;
	do {
		y = (rand() % LEVEL_H) + 1;
		x = (rand() % LEVEL_W) + 1;
	} while (!is_some_char(win, y, x, FLOOR_CHAR));
	mvwaddch(win, y, x, SNAK_CHAR);
	sn -> locy = y;
	sn -> locx = x;
	sn -> exists = true;
	return sn;
}

void draw_snak(WINDOW* win, struct Snak* sn) {
	mvwaddch(win, sn -> locy, sn -> locx, SNAK_CHAR);
}

void collect_snak(struct Player* pl, struct Snak* sn) {
	if (pl -> locy == sn -> locy && pl -> locx == sn -> locx) {
		sn -> exists = false;
		grow_player(pl);
		free(sn);
	}
}

/* draw the scoreboard */
void draw_score(WINDOW* win, struct Player* pl) {
	mvwprintw(win, 1, 2, "%d", pl -> score);
}

/* draw game over screen */
void game_over() {
	WINDOW* game_over_win = init_win(3,13,LEVEL_H/2-1, LEVEL_W/2-6);
	box(game_over_win, 0, 0);
	mvwaddstr(game_over_win, 1, 2, "GAME OVER");
	wrefresh(game_over_win);
	nodelay(stdscr, false);
	napms(500);
	char ch = getch();
}

/* where all the stuff happens */

void game_loop() {
	WINDOW* game_win = init_win(LEVEL_H+2, LEVEL_W+2, 0, 0);
	WINDOW* score_win = init_win(3, 7, LEVEL_H+2, 0);
	struct Player* pl = init_player();
	struct Snak* sn = init_snak(game_win);
	char ch;
	while (true) {
		/* check player input */
		ch = getch();
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
		werase(game_win);

		wborder(game_win, '#', '#', '#', '#', '#', '#', '#', '#');
		box(score_win, 0, 0);
		draw_snak(game_win, sn);
		draw_player(game_win, pl);
		draw_score(score_win, pl);

		wrefresh(game_win);
		wrefresh(score_win);

		move_player(game_win, pl);
		if (!(pl -> alive)) break;
		if (!(sn -> exists)) sn = init_snak(game_win);
		collect_snak(pl, sn);

		napms(1000/FRAME_RATE);
	}
	/* free player tail memory */
	free(pl -> taily);
	free(pl -> tailx);
	free(pl);
	pl = NULL;

	/* show game over screen */
	game_over();

	/* kill windows */
	kill_win(game_win);
	kill_win(score_win);
}

int main(void) {
  	start_ncurses();
	game_loop();
  	stop_ncurses();
}
