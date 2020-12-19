#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include "types.h"
#include "util_funs.h"
#include "print_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))
#define TRY(expr) if (expr) ERROR(#expr)
#define UNUSED(x) (void)(x)
#define SET_GAME_MODE(mode) TRY(setenv("IS_GAME_MODE", #mode, 1) < 0)

/**
 * Move to a different room if possible.
 */
void move_to(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Pick up an object in the current room if the inventory is not full.
 */
void pick_up(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Drop an object into the current room if there is space available.
 */
void drop(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Save the game to a file.
 */
void save(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Find a path to a different room using worker threads.
 */
void find_path(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Worker thread function for path finding.
 */
void *find_path_worker(void *voidArgs);

/**
 * Quit the game and return to the main menu.
 */
void quit(gameState_t *game, WINDOW *win);
