#include <linux/limits.h>
#include <ncurses.h>
#include "game_funs.h"

#define INVALID_CMD -1
#define OK_CMD       0
#define EXIT_CMD     1
#define QUIT_CMD     2

typedef enum cmdType {
    Map_from_dir_tree,
    Generate_random_map,
    Start_game,
    Load_game,
    Exit,

    Move_to,
    Pick_up,
    Drop,
    Save,
    Find_path,
    Quit,
} cmdType_t;

typedef struct command {
    cmdType_t type;
    char *args;
} command_t;

void parse_args(int argc, char **argv);

void set_backup(char *path);

int exec_command(char *cmd, WINDOW *win, gameState_t *game);
