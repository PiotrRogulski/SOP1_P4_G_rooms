#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>

#define SET_GAME_MODE(mode) setenv("IS_GAME_MODE", #mode, 1)

typedef struct object {
    int id;
    int room;
} object_t;

typedef struct room {
    int id;
    object_t *objects;
} room_t;

typedef struct gameState {
    char *roomsMap;
    room_t *rooms;
    int playerPosition;
    int n;
} gameState_t;

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f);
void map_from_dir_tree(char *cmd, WINDOW *win);
void generate_random_map(char *cmd, WINDOW *win);
void start_game(char *cmd, WINDOW *win, gameState_t *game);
void load_game(void);

void move_to(void);
void pick_up(void);
void drop(void);
void save(void);
void find_path(void);
void quit(void);
