#include <stdlib.h>
#include <unistd.h>

#define SET_GAME_MODE(mode) setenv("IS_GAME_MODE", #mode, 1)

void map_from_dir_tree(void);
void generate_random_map(void);
void start_game(void);
void load_game(void);

void move_to(void);
void pick_up(void);
void drop(void);
void save(void);
void find_path(void);
void quit(void);
