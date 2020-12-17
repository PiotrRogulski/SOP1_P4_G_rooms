#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
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
        if (setenv("GAME_AUTOSAVE", "/home/piotr/.game-autosave", 1))
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
            return OK_CMD;
        }

        if (strncmp(cmd, "move-to", 7) == 0) {
            move_to(cmd, game, win);
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
            save(cmd, game, win);
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
            start_game(cmd, game, win);
            return OK_CMD;
        }
        if (strncmp(cmd, "load-game", 9) == 0) {
            load_game(cmd, game, win);
            return OK_CMD;
        }
    }

    return INVALID_CMD;
}

void *alarm_generator(void *voidArgs) {
    gameState_t *game = voidArgs;
    timespec_t t = { 6, 0 };

    while (1) {
        int sig_no = sigtimedwait(game->mask, NULL, &t);
        if (sig_no > 0)
            continue;
        pthread_kill(game->auto_save_tid, SIGALRM);
    }
    return NULL;
}

void *auto_save_game(void *voidArgs) {
    pthread_cleanup_push(unlock_mutexes, (gameState_t*) voidArgs);

    gameState_t *game = voidArgs;
    unsigned n = game->n;
    char *path = getenv("GAME_AUTOSAVE");
    int f;

    int sig_no;

    while (1) {
        if (sigwait(game->mask, &sig_no))
            ERROR("Couldn't wait for signal");
        switch (sig_no) {
            case SIGALRM:
                f = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777);
                if (f == -1)
                    ERROR("Couldn't open file for autosave");
                pthread_mutex_lock(game->game_mutex);

                mvwprintw(game->win, getmaxy(game->win) - 1, getmaxx(game->win) / 2 - 12, " Performing auto-save ");

                if (write(f, &n, sizeof(unsigned)) < 0)
                    ERROR("Couldn't write n");
                if (write(f, game->rooms_map, n*n * sizeof(char)) < 0)
                    ERROR("Couldn't write graph to file");

                if (write(f, &game->player_position, sizeof(unsigned)) < 0)
                    ERROR("Couldn't write player position");
                if (write(f, &game->num_player_objects, sizeof(unsigned)) < 0)
                    ERROR("Couldn't write inventory object count");
                for (unsigned i = 0; i < game->num_player_objects; i++)
                    if (write(f, game->player_objects[i], sizeof(object_t)) < 0)
                        ERROR("Couldn't write object");

                for (unsigned i = 0; i < n; i++) {
                    if (write(f, &i, sizeof(unsigned)) < 0)
                        ERROR("Couldn't write room id");
                    if (write(f, &game->rooms[i].num_existing_objects, sizeof(unsigned)) < 0)
                        ERROR("Couldn't write object count");
                    if (write(f, &game->rooms[i].num_assigned_objects, sizeof(unsigned)) < 0)
                        ERROR("Couldn't write assigned object count");
                    for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++)
                        if (write(f, game->rooms[i].objects[j], sizeof(object_t)) < 0)
                            ERROR("Couldn't write object");
                }

                wborder(game->win, 0, 0, 0, 0, 0, 0, 0, 0);
                wrefresh(game->win);

                close(f);
                pthread_mutex_unlock(game->game_mutex);
                break;
            default:
                ERROR("Unexpected signal");
        }
    }
    pthread_cleanup_pop(0);
    return NULL;
}

void unlock_mutexes(void *voidArgs) {
    pthread_mutex_unlock(((gameState_t*) voidArgs)->game_mutex);
}
