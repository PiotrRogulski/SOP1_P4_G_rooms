#include <linux/limits.h>
#include <ncurses.h>

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

command_t *parse_command(char *cmd);
