#pragma once

#include <errno.h>
#include <ftw.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wordexp.h>
#include <sys/stat.h>
#include "print_funs.h"
#include "types.h"
#include "util_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))
#define TRY(expr) if (expr) ERROR(#expr)
#define UNUSED(x) (void)(x)
#define MAXFD 20
#define SET_GAME_MODE(mode) TRY(setenv("IS_GAME_MODE", #mode, 1) < 0)

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f);

/**
 * Create a map from files in the specified directory.
 */
void map_from_dir_tree(char *cmd);

/**
 * Generate a random connected graph of size n.
 */
void generate_random_map(char *cmd, WINDOW *win);

/**
 * Initialize the game structure woth random objects on a given map.
 */
void start_game(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Load a saved game from a file.
 */
void load_game(char *cmd, gameState_t *game, WINDOW *win);
