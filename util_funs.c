#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "util_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))

void parse_args(int argc, char **argv) {
    int c;
    char *arg = NULL;
    while ((c = getopt(argc, argv, "b:")) != -1) {
        switch (c) {
            case 'b':
                arg = malloc(strlen(optarg) + 1);
                if (arg == NULL)
                    ERROR("Couldn't allocate memory");
                strcpy(arg, optarg);
                break;
            case '?':
            default:
                fprintf(stderr, "Unrecognized option: %c\n", c);
        }
    }
    set_backup(arg);
    free(arg);
}

void set_backup(char *path) {
    if (path != NULL) {
        if (setenv("GAME_AUTOSAVE", path, 1))
            ERROR("Couldn't set GAME_AUTOSAVE");
        return;
    }
    char *env_backup_path = getenv("GAME_AUTOSAVE");
    if (env_backup_path == NULL)
        if (setenv("GAME_AUTOSAVE", "~/.game-autosave", 1))
            ERROR("Couldn't set GAME_AUTOSAVE");
}

int exec_command(char *cmd, WINDOW *win, gameState_t *game) {
    char *isGameModeStr = getenv("IS_GAME_MODE");
    int isGameMode = 1;
    if (isGameModeStr == NULL || atoi(isGameModeStr) == 0)
        isGameMode = 0;

    char *firstSpace = strchr(cmd, ' ');

    if (isGameMode) {
        if (firstSpace == NULL) {
            if (strncmp(cmd, "quit", 4) != 0) {
                return INVALID_CMD;
            }
            quit(game, win);
            return QUIT_CMD;
        }

        if (strncmp(cmd, "move-to", 7) == 0) {
            move_to(cmd, win, game);
            return OK_CMD;
        }
        if (strncmp(cmd, "pick-up", 7) == 0) {
            pick_up(cmd, game, win);
            return OK_CMD;
        }
        if (strncmp(cmd, "drop", 4) == 0) {
            drop(cmd, game, win);
            return OK_CMD;
        }
        if (strncmp(cmd, "save", 4) == 0) {
            save();
            return OK_CMD;
        }
        if (strncmp(cmd, "find-path", 9) == 0) {
            find_path(cmd, game, win);
            return OK_CMD;
        }
    } else { // in menu
        if (firstSpace == NULL) {
            if (strncmp(cmd, "exit", 4) != 0) {
                return INVALID_CMD;
            }
            return EXIT_CMD;
        }

        if (strncmp(cmd, "map-from-dir-tree", 17) == 0) {
            map_from_dir_tree(cmd, win);
            return OK_CMD;
        }
        if (strncmp(cmd, "generate-random-map", 19) == 0) {
            generate_random_map(cmd, win);
            return OK_CMD;
        }
        if (strncmp(cmd, "start-game", 10) == 0) {
            start_game(cmd, win, game);
            return OK_CMD;
        }
        if (strncmp(cmd, "load-game", 9) == 0) {
            load_game();
            return OK_CMD;
        }
    }

    return INVALID_CMD;
}
