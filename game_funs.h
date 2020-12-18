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
#define TRY(expr) if (expr) ERROR(#expr)
#define UNUSED(x) (void)(x)
#define SET_GAME_MODE(mode) setenv("IS_GAME_MODE", #mode, 1)

void move_to(char *cmd, gameState_t *game, WINDOW *win);
void pick_up(char *cmd, gameState_t *game, WINDOW *win);
void drop(char *cmd, gameState_t *game, WINDOW *win);
void save(char *cmd, gameState_t *game, WINDOW *win);
void find_path(char *cmd, gameState_t *game, WINDOW *win);
void *find_path_worker(void *voidArgs);
void quit(gameState_t *game, WINDOW *win);
