#include <ftw.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "print_funs.h"
#include "types.h"

void print_game(gameState_t *game, WINDOW *win) {
    werase(win);
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

    print_table(game->rooms_map, game->n, win);
    print_curr_room(game->player_position, game->n, win);
    print_objects(game, win);
    print_inventory(game, win);

    wrefresh(win);
}

/**
 * Prints connections between the rooms
 */
void print_table(char *table, unsigned n, WINDOW *win) {
    for (unsigned i = 0; i < n; i++) {
        mvwprintw(win, 3, 7 + 3*i, "%-2u", i);
        mvwprintw(win, 7 + 2*i, 2, "%2u", i);
    }

    mvwhline(win, 5, 2, ACS_HLINE, 5 + 3*n);
    mvwvline(win, 2, 5, ACS_VLINE, 5 + 2*n);
    mvwaddch(win, 5, 5, ACS_PLUS);

    for (unsigned i = 0; i < n; i++) {
        for (unsigned j = 0; j < n; j++) {
            if (table[i * n + j] == 1)
                wattron(win, COLOR_PAIR(2));
            else
                wattron(win, COLOR_PAIR(1));
            mvwaddch(win, 7 + 2*i, 7 + 3*j, ' ');
        }
    }
    wattroff(win, COLOR_PAIR(1) | COLOR_PAIR(2));
}

/**
 * Print current position of the player
 */
void print_curr_room(unsigned curr, unsigned n, WINDOW *win) {
    mvwprintw(win, 9 + 2*n, 17, "  ");
    mvwprintw(win, 9 + 2*n, 3, "Current room: %u", curr);
}

/**
 * Prints objects in each room
 */
void print_objects(gameState_t *game, WINDOW *win) {
    unsigned n = game->n;
    unsigned lines_printed = 0;

    unsigned y = 3;
    unsigned x = 12 + 3*n;

    mvwprintw(win, y, x, "Objects in the game: ");
    for (unsigned i = 0; i < n; i++) {
        if (game->rooms[i].num_existing_objects == 0)
            continue;
        mvwprintw(win, y + 2 + lines_printed, x + 2, "%2u:", i);
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++) {
            mvwprintw(win, y + 2 + lines_printed, x + 8, "%-2u assigned to %u", game->rooms[i].objects[j]->id, game->rooms[i].objects[j]->assigned_room);
            lines_printed++;
        }
        lines_printed++;
    }
}

/**
 * Prints objects in player's inventory
 */
void print_inventory(gameState_t *game, WINDOW *win) {
    unsigned n = game->n;

    unsigned y = 3;
    unsigned x = 45 + 3*n;

    mvwprintw(win, y, x, "Player's inventory:");
    y += 2;
    if (game->num_player_objects == 0) {
        mvwprintw(win, y, x+4, "EMPTY");
    } else {
        for (unsigned int i = 0; i < game->num_player_objects; i++)
            mvwprintw(win, y + i, x+4, "%-2u assigned to %u", game->player_objects[i]->id, game->player_objects[i]->assigned_room);
    }
    wrefresh(win);
}
