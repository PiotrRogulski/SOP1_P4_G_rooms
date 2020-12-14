#pragma once

#include <linux/limits.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include "game_funs.h"
#include "types.h"

#define INVALID_CMD -1
#define OK_CMD       0
#define EXIT_CMD     1
#define QUIT_CMD     2

void parse_args(int argc, char **argv);

void set_backup(char *path);

int exec_command(char *cmd, WINDOW *win, pthread_mutex_t *winMutex, gameState_t *game);

// void show_error(char *msg, WINDOW *win, pthread_mutex_t *winMutex);

// void *show_error_work(void *voidArgs);
