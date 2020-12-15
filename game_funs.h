#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <pthread.h>
#include "types.h"
#include "util_funs.h"

#define SET_GAME_MODE(mode) setenv("IS_GAME_MODE", #mode, 1)

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f);
void map_from_dir_tree(char *cmd, WINDOW *win);
void generate_random_map(char *cmd, WINDOW *win);
void start_game(char *cmd, WINDOW *win, gameState_t *game);
void load_game(void);

void move_to(char *cmd, WINDOW *win, gameState_t *game);
void pick_up(char *cmd, gameState_t *game, WINDOW *win);
void drop(void);
void save(void);
void find_path(void);
void quit(gameState_t *game, WINDOW *win);

void print_game(gameState_t *game, WINDOW *win);
void print_table(char *table, unsigned n, WINDOW *win);
void print_curr_room(unsigned curr, unsigned n, WINDOW *win);
void print_objects(gameState_t *game, WINDOW *win);
void print_inventory(gameState_t *game, WINDOW *win);
