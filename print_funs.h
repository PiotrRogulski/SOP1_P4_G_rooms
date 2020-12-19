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

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))

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
