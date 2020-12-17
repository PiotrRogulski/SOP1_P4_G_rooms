#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <pthread.h>
#include "types.h"
#include "util_funs.h"
#include "print_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))
#define UNUSED(x) (void)(x)
#define MAXFD 20
#define SET_GAME_MODE(mode) setenv("IS_GAME_MODE", #mode, 1)

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f);

/**
 * Creates a map from files in the specified directory
 * Not finished yet
 */
void map_from_dir_tree(char *cmd, WINDOW *win);

/**
 * Generates a random connected graph of size n
 */
void generate_random_map(char *cmd, WINDOW *win);

/**
 * Initializes the game structure woth random objects on a given map
 */
void start_game(char *cmd, gameState_t *game, WINDOW *win);

/**
 * Loads a saved game from a file
 */
void load_game(char *cmd, gameState_t *game, WINDOW *win);
