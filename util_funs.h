#pragma once

#include <linux/limits.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include "game_funs.h"
#include "menu_funs.h"
#include "types.h"

#define INVALID_CMD -1
#define OK_CMD       0
#define EXIT_CMD     1

void parse_args(int argc, char **argv);

void set_backup(char *path);

int exec_command(char *cmd, WINDOW *win, gameState_t *game);
