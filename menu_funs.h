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
void map_from_dir_tree(char *cmd, WINDOW *win);
void generate_random_map(char *cmd, WINDOW *win);
void start_game(char *cmd, gameState_t *game, WINDOW *win);
void load_game(char *cmd, gameState_t *game, WINDOW *win);
