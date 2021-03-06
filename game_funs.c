/**
 * Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów
 * uczenia się z przedmiotu SOP1 została wykonana przeze mnie samodzielnie.
 * Piotr Rogulski
 * 305867
 */

#include "game_funs.h"

void move_to(char *cmd, gameState_t *game, WINDOW *win) {
    pthread_mutex_lock(game->game_mutex);

    unsigned x;
    unsigned curr = game->player_position;
    unsigned n = game->n;
    if (sscanf(cmd, "move-to %u", &x) <= 0)
        return;

    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

    if (x < n && game->rooms_map[x * n + curr] == 1) {
        game->player_position = x;
        print_msg(win, "Moved to room %u", x);
        print_curr_room(x, n, win);
    } else if (x >= n) {
        print_msg(win, "Room %u doesn't exist", x);
    } else {
        print_msg(win, "Cannot move to room %u", x);
    }

    pthread_mutex_unlock(game->game_mutex);

    wrefresh(win);
}

void pick_up(char *cmd, gameState_t *game, WINDOW *win) {
    unsigned y;
    if (sscanf(cmd, "pick-up %u", &y) <= 0)
        return;

    pthread_mutex_lock(game->game_mutex);

    if (game->num_player_objects == 2) {
        print_msg(win, "Cannot pick up object: inventory full");
        wrefresh(win);
        return;
    }

    object_t *obj = NULL;
    unsigned i;
    for (i = 0; i < game->rooms[game->player_position].num_existing_objects; i++) {
        if (game->rooms[game->player_position].objects[i]->id == y) {
            obj = game->rooms[game->player_position].objects[i];
            break;
        }
    }

    if (obj == NULL) {
        print_msg(win, "Object with id %u doesn't exist here", y);
        wrefresh(win);
        pthread_mutex_unlock(game->game_mutex);
        return;
    }

    if (i == 0)
        game->rooms[game->player_position].objects[0] = game->rooms[game->player_position].objects[1];

    game->rooms[game->player_position].objects[1] = NULL;
    game->rooms[game->player_position].num_existing_objects--;
    game->player_objects[game->num_player_objects++] = obj;

    pthread_mutex_unlock(game->game_mutex);
    print_msg(win, "Picked up object #%u", obj->id);
    print_game(game, win);
}

void drop(char *cmd, gameState_t *game, WINDOW *win) {
    pthread_mutex_lock(game->game_mutex);

    unsigned pos = game->player_position;

    unsigned z;
    if (sscanf(cmd, "drop %u", &z) <= 0)
        return;

    if (game->num_player_objects == 0) {
        print_msg(win, "Empty inventory");
        pthread_mutex_unlock(game->game_mutex);
        return;
    }

    object_t *obj = NULL;
    unsigned i;
    for (i = 0; i < game->num_player_objects; i++) {
        if (game->player_objects[i]->id == z) {
            obj = game->player_objects[i];
            break;
        }
    }

    if (obj == NULL) {
        print_msg(win, "Object with id %u not in inventory", z);
        pthread_mutex_unlock(game->game_mutex);
        return;
    }

    if (game->rooms[pos].num_existing_objects == 2) {
        print_msg(win, "Already two objects in this room");
        pthread_mutex_unlock(game->game_mutex);
        return;
    }

    if (i == 0) {
        game->player_objects[0] = game->player_objects[1];
    }

    game->player_objects[1] = NULL;
    game->num_player_objects--;
    game->rooms[pos].objects[game->rooms[pos].num_existing_objects++] = obj;

    pthread_mutex_unlock(game->game_mutex);
    print_msg(win, "Dropped object #%u", obj->id);
    print_game(game, win);
}

void save(char *cmd, gameState_t *game, WINDOW *win) {
    pthread_mutex_lock(game->game_mutex);

    char path[PATH_MAX];
    unsigned n = game->n;

    if (sscanf(cmd, "save %s", path) <= 0)
        return;

    char *new_path = expand_path(path);
    strcpy(path, new_path);
    free(new_path);

    int file;
    TRY((file = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0);

    TRY(write(file, &n, sizeof(unsigned)) < 0);
    TRY(write(file, game->rooms_map, n*n * sizeof(char)) < 0);

    TRY(write(file, &game->player_position, sizeof(unsigned)) < 0);
    TRY(write(file, &game->num_player_objects, sizeof(unsigned)) < 0);
    for (unsigned i = 0; i < game->num_player_objects; i++)
        TRY(write(file, game->player_objects[i], sizeof(object_t)) < 0);

    for (unsigned i = 0; i < n; i++) {
        TRY(write(file, &i, sizeof(unsigned)) < 0);
        TRY(write(file, &game->rooms[i].num_existing_objects, sizeof(unsigned)) < 0);
        TRY(write(file, &game->rooms[i].num_assigned_objects, sizeof(unsigned)) < 0);
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++)
            TRY(write(file, game->rooms[i].objects[j], sizeof(object_t)) < 0);
    }

    pthread_kill(game->alarm_generator_tid, SIGALRM);

    close(file);
    pthread_mutex_unlock(game->game_mutex);
    print_game(game, win);
}

void find_path(char *cmd, gameState_t *game, WINDOW *win) {
    unsigned k, x;

    if (sscanf(cmd, "find-path %u %u", &k, &x) <= 0)
        return;

    pthread_t tids[k];
    pathFind_t args[k];

    for (unsigned i = 0; i < k; i++) {
        args[i].game = game;
        args[i].seed = rand();
        args[i].destination = x;
        TRY(pthread_create(&tids[i], NULL, find_path_worker, &args[i]));
    }

    for (unsigned i = 0; i < k; i++)
        TRY(pthread_join(tids[i], NULL));

    unsigned shortest_id = 0;
    unsigned shortest_length = args[0].length;

    for (unsigned i = 1; i < k; i++) {
        if (args[i].length < shortest_length) {
            shortest_length = args[i].length;
            shortest_id = i;
        }
    }

    print_game(game, win);

    if (shortest_length == 2000) {
        print_msg(win, "Couldn't find a path");
        for (unsigned i = 0; i < k; i++)
            free(args[i].path);
        return;
    }

    mvwprintw(win, getmaxy(win) - 3, 2, "Found path from %u to %u: ", game->player_position, x);
    for (unsigned i = 0; i < shortest_length; i++)
        mvwprintw(win, getmaxy(win) - 3, 35 + 3*i, "%2u ", args[shortest_id].path[i]);
    wrefresh(win);

    for (unsigned i = 0; i < k; i++)
        free(args[i].path);
}

void *find_path_worker(void *voidArgs) {
    pathFind_t *args = voidArgs;

    gameState_t *game = args->game;
    unsigned n = game->n;
    unsigned seed = args->seed;
    unsigned length = 0;
    unsigned curr = game->player_position;
    unsigned destination = args->destination;
    unsigned *path;

    TRY((path = malloc(1000 * sizeof(unsigned))) == NULL);

    while (curr != destination && length < 1000) {
        unsigned next;
        pthread_mutex_lock(game->game_mutex);
        do {
            next = rand_r(&seed) % n;
        } while (game->rooms_map[next * n + curr] == 0);
        pthread_mutex_unlock(game->game_mutex);
        path[length++] = next;
        curr = next;
    }

    args->length = (curr == destination ? length : 2000);
    args->path = path;

    return NULL;
}

void quit(gameState_t *game, WINDOW *win) {
    SET_GAME_MODE(0);

    pthread_mutex_lock(game->game_mutex);

    for (unsigned i = 0; i < game->n; i++) {
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++) {
            free(game->rooms[i].objects[j]);
        }
    }

    for (unsigned i = 0; i < game->num_player_objects; i++)
        free(game->player_objects[i]);

    free(game->rooms);
    free(game->rooms_map);

    TRY(pthread_cancel(game->auto_save_tid));
    TRY(pthread_cancel(game->alarm_generator_tid));
    TRY(pthread_cancel(game->swap_objects_tid));
    TRY(pthread_cancel(game->user_signal_catcher_tid));

    pthread_mutex_unlock(game->game_mutex);

    werase(win);
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
    wrefresh(win);
}
