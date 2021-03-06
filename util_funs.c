/**
 * Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów
 * uczenia się z przedmiotu SOP1 została wykonana przeze mnie samodzielnie.
 * Piotr Rogulski
 * 305867
 */

#include "util_funs.h"

void parse_args(int argc, char **argv) {
    int c;
    char *arg = NULL;
    while ((c = getopt(argc, argv, "b:")) != -1) {
        switch (c) {
            case 'b':
                TRY((arg = malloc(strlen(optarg) + 1)) == NULL);
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
    char default_path[] = "~/.game-autosave";
    char *new_path;

    if (path != NULL) {
        new_path = expand_path(path);
        TRY(setenv("GAME_AUTOSAVE", new_path, 1));
        free(new_path);
    } else {
        char *env_backup_path = getenv("GAME_AUTOSAVE");
        if (env_backup_path == NULL) {
            new_path = expand_path(default_path);
            TRY(setenv("GAME_AUTOSAVE", new_path, 1));
            free(new_path);
        }
    }
}

int exec_command(char *cmd, WINDOW *win, gameState_t *game) {
    char *is_game_mode_str = getenv("IS_GAME_MODE");
    int is_game_mode = 1;
    if (is_game_mode_str == NULL || atoi(is_game_mode_str) == 0)
        is_game_mode = 0;

    if (is_game_mode) {
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
        if (strncmp(cmd, "quit", 4) == 0) {
            quit(game, win);
            return OK_CMD;
        }
    } else { // in menu
        if (strncmp(cmd, "map-from-dir-tree", 17) == 0) {
            map_from_dir_tree(cmd);
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
        if (strncmp(cmd, "exit", 4) == 0) {
            return EXIT_CMD;
        }
    }

    return INVALID_CMD;
}

void *alarm_generator(void *voidArgs) {
    gameState_t *game = voidArgs;
    timespec_t t = { 60, 0 };
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    while (1) {
        int sig_no = sigtimedwait(&mask, NULL, &t);
        if (sig_no > 0)
            continue;
        TRY(pthread_kill(game->auto_save_tid, SIGALRM));
    }

    return NULL;
}

void *user_signal_catcher(void *voidArgs) {
    gameState_t *game = voidArgs;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    int sig_no;

    while (1) {
        TRY(sigwait(&mask, &sig_no));
        if (sig_no > 0)
            TRY(pthread_kill(game->swap_objects_tid, SIGUSR1));
    }

    return NULL;
}

void *auto_save_game(void *voidArgs) {
    pthread_cleanup_push(unlock_mutexes, voidArgs);

    gameState_t *game = voidArgs;
    unsigned n = game->n;

    char *path = getenv("GAME_AUTOSAVE");
    char *new_path = expand_path(path);
    strcpy(path, new_path);
    free(new_path);

    int f;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    int sig_no;

    while (1) {
        TRY(sigwait(&mask, &sig_no));
        switch (sig_no) {
            case SIGALRM:
                TRY((f = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) == -1);
                pthread_mutex_lock(game->game_mutex);

                print_msg(game->win, "Performing auto-save");

                TRY(write(f, &n, sizeof(unsigned)) < 0);
                TRY(write(f, game->rooms_map, n*n * sizeof(char)) < 0);

                TRY(write(f, &game->player_position, sizeof(unsigned)) < 0);
                TRY(write(f, &game->num_player_objects, sizeof(unsigned)) < 0);
                for (unsigned i = 0; i < game->num_player_objects; i++)
                    TRY(write(f, game->player_objects[i], sizeof(object_t)) < 0);

                for (unsigned i = 0; i < n; i++) {
                    TRY(write(f, &i, sizeof(unsigned)) < 0);
                    TRY(write(f, &game->rooms[i].num_existing_objects, sizeof(unsigned)) < 0);
                    TRY(write(f, &game->rooms[i].num_assigned_objects, sizeof(unsigned)) < 0);
                    for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++)
                        TRY(write(f, game->rooms[i].objects[j], sizeof(object_t)) < 0);
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

void *swap_objects(void *voidArgs) {
    pthread_cleanup_push(unlock_mutexes, voidArgs);

    gameState_t *game = voidArgs;
    unsigned seed = game->swap_seed;
    unsigned n = game->n;
    object_t **obj1 = NULL, **obj2 = NULL, *temp;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    int sig_no;

    while (1) {
        TRY(sigwait(&mask, &sig_no));
        switch (sig_no) {
            case SIGUSR1:
                pthread_mutex_lock(game->game_mutex);

                do {
                    unsigned room_id = rand_r(&seed) % n;
                    if (game->rooms[room_id].num_existing_objects > 0) {
                        unsigned obj_id = rand_r(&seed) % game->rooms[room_id].num_existing_objects;
                        obj1 = &game->rooms[room_id].objects[obj_id];
                    }
                } while (obj1 == NULL);

                do {
                    unsigned room_id = rand_r(&seed) % n;
                    if (game->rooms[room_id].num_existing_objects > 0) {
                        unsigned obj_id = rand_r(&seed) % game->rooms[room_id].num_existing_objects;
                        obj2 = &game->rooms[room_id].objects[obj_id];
                    }
                } while ((obj2 == NULL) != (obj2 == obj1));

                temp = *obj1;
                *obj1 = *obj2;
                *obj2 = temp;

                pthread_mutex_unlock(game->game_mutex);

                WINDOW *win = game->win;
                print_game(game, win);
                print_msg(win, "Swapped objects #%u and #%u", (*obj1)->id, (*obj2)->id);
                wrefresh(win);

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

char *expand_path(char *path) {
    wordexp_t p;
    TRY(wordexp(path, &p, 0));
    char *new_path;
    TRY(asprintf(&new_path, "%s", p.we_wordv[0]) < 0);
    wordfree(&p);
    return new_path;
}
