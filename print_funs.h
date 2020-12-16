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

void print_game(gameState_t *game, WINDOW *win);
void print_table(char *table, unsigned n, WINDOW *win);
void print_curr_room(unsigned curr, unsigned n, WINDOW *win);
void print_objects(gameState_t *game, WINDOW *win);
void print_inventory(gameState_t *game, WINDOW *win);
