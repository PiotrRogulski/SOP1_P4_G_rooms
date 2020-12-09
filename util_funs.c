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

command_t *parse_command(char *cmd) {
    char *isGameModeStr = getenv("IS_GAME_MODE");
    int isGameMode = 1;
    if (isGameModeStr == NULL || atoi(isGameModeStr) == 0)
        isGameMode = 0;

    command_t *ret = malloc(sizeof(command_t));
    if (ret == NULL)
        ERROR("Couldn't allocate command");
    char *firstSpace = strchr(cmd, ' ');

    if (isGameMode) {
        if (firstSpace == NULL) {
            if (strcmp(cmd, "quit") != 0) {
                free(ret);
                return NULL;
            }
            ret->type = Quit;
            ret->args = NULL;
            return ret;
        }
        *firstSpace = 0;
        char *args = firstSpace + 1;
        ret->args = malloc(strlen(args) + 1);
        if (ret->args == NULL)
            ERROR("Couldn't allocate command args");
        strcpy(ret->args, args);
        if (strcmp(cmd, "move-to") == 0) {
            ret->type = Move_to;
            return ret;
        }
        if (strcmp(cmd, "pick-up") == 0) {
            ret->type = Pick_up;
            return ret;
        }
        if (strcmp(cmd, "drop") == 0) {
            ret->type = Drop;
            return ret;
        }
        if (strcmp(cmd, "save") == 0) {
            ret->type = Save;
            return ret;
        }
        if (strcmp(cmd, "find-path") == 0) {
            ret->type = Find_path;
            return ret;
        }
    } else { // in menu
        if (firstSpace == NULL) {
            if (strcmp(cmd, "exit") != 0) {
                free(ret);
                return NULL;
            }
            ret->type = Exit;
            ret->args = NULL;
            return ret;
        }
        *firstSpace = 0;
        char *args = firstSpace + 1;
        ret->args = malloc(strlen(args) + 1);
        if (ret->args == NULL)
            ERROR("Couldn't allocate command args");
        strcpy(ret->args, args);

        if (strcmp(cmd, "map-from-dir-tree") == 0) {
            ret->type = Map_from_dir_tree;
            return ret;
        }
        if (strcmp(cmd, "generate-random-map") == 0) {
            ret->type = Generate_random_map;
            return ret;
        }
        if (strcmp(cmd, "start-game") == 0) {
            ret->type = Start_game;
            return ret;
        }
        if (strcmp(cmd, "load-game") == 0) {
            ret->type = Load_game;
            return ret;
        }
    }
    printf("%d\n", ret->type);
    // Invalid command
    free(ret->args);
    free(ret);
    return NULL;
}
