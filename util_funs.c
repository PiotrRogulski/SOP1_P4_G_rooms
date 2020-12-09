#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "util_funs.h"
#include "game_funs.h"

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

int exec_command(char *cmd, WINDOW *mainWin) {
    char *isGameModeStr = getenv("IS_GAME_MODE");
    int isGameMode = 1;
    if (isGameModeStr == NULL || atoi(isGameModeStr) == 0)
        isGameMode = 0;

    char *firstSpace = strchr(cmd, ' ');

    if (isGameMode) {
        if (firstSpace == NULL) {
            if (strcmp(cmd, "quit") != 0) {
                return INVALID_CMD;
            }
            return QUIT_CMD;
        }
        *firstSpace = 0;
        char *args = firstSpace + 1;

        if (strcmp(cmd, "move-to") == 0) {
            move_to();
            return OK_CMD;
        }
        if (strcmp(cmd, "pick-up") == 0) {
            pick_up();
            return OK_CMD;
        }
        if (strcmp(cmd, "drop") == 0) {
            drop();
            return OK_CMD;
        }
        if (strcmp(cmd, "save") == 0) {
            save();
            return OK_CMD;
        }
        if (strcmp(cmd, "find-path") == 0) {
            find_path();
            return OK_CMD;
        }
    } else { // in menu
        if (firstSpace == NULL) {
            if (strcmp(cmd, "exit") != 0) {
                return INVALID_CMD;
            }
            return EXIT_CMD;
        }
        *firstSpace = 0;
        char *args = firstSpace + 1;
        system("touch ./test");

        if (strcmp(cmd, "map-from-dir-tree") == 0) {
            map_from_dir_tree();
            return OK_CMD;
        }
        if (strcmp(cmd, "generate-random-map") == 0) {
            generate_random_map();
            return OK_CMD;
        }
        if (strcmp(cmd, "start-game") == 0) {
            start_game();
            return OK_CMD;
        }
        if (strcmp(cmd, "load-game") == 0) {
            load_game();
            return OK_CMD;
        }
    }

    return INVALID_CMD;
}
