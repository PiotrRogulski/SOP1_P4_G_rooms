#include <stdlib.h>
#include <unistd.h>

#define SET_GAME_MODE(mode) setenv("IS_GAME_MODE", #mode, 1)

void map_from_dir_tree(char *cmd, WINDOW *win);
void generate_random_map(char *cmd, WINDOW *win);
void start_game(void);
void load_game(void);

void move_to(void);
void pick_up(void);
void drop(void);
void save(void);
void find_path(void);
void quit(void);
