#pragma once

#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "types.h"
#include "util_funs.h"

/**
 * Print current game state
 */
void print_game(gameState_t *game, WINDOW *win);

/**
 * Print connections between the rooms.
 */
void print_table(char *table, unsigned n, WINDOW *win);

/**
 * Print current player position.
 */
void print_curr_room(unsigned curr, unsigned n, WINDOW *win);

/**
 * Print objects in each room.
 */
void print_objects(gameState_t *game, WINDOW *win);

/**
 * Print objects in player's inventory.
 */
void print_inventory(gameState_t *game, WINDOW *win);

/**
 * Print a message when all objects are in the correct room.
 */
void print_game_complete(gameState_t *game, WINDOW *win);

/**
 * Print a message at the bottom of the screen.
 */
void print_msg(WINDOW *win, char *fmt, ...);
