#include "menu_funs.h"

#define TMP_FILE "/tmp/dir_tree.txt"

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f) {
    UNUSED(name);
    UNUSED(s);

    if (type != FTW_D)
        return 0;

    int id;
    TRY((id = open(TMP_FILE, O_WRONLY | O_CREAT | O_APPEND, 0777)) < 0);

    TRY(write(id, &f->level, sizeof(f->level)) < 0);

    close(id);
    return 0;
}

void map_from_dir_tree(char* cmd) {
    char dir_path[PATH_MAX];
    char file_path[PATH_MAX];
    if (sscanf(cmd, "map-from-dir-tree %s %s", dir_path, file_path) <= 0)
        return;

    char *new_dir_path = expand_path(dir_path);
    strcpy(dir_path, new_dir_path);
    free(new_dir_path);

    char *new_file_path = expand_path(file_path);
    strcpy(file_path, new_file_path);
    free(new_file_path);

    TRY(remove(TMP_FILE) && errno != ENOENT);
    TRY(nftw(dir_path, walk_print, MAXFD, FTW_PHYS) < 0);

    struct stat *s = malloc(sizeof(struct stat));
    TRY(stat(TMP_FILE, s) != 0);

    unsigned n = s->st_size / sizeof(int);

    char *map = calloc(n*n, sizeof(char));
    int *list = calloc(n + 1, sizeof(int));

    int f;
    TRY((f = open(TMP_FILE, O_RDONLY)) < 0);
    int c;
    int r;
    unsigned i = 0;
    list[0] = 0;
    while ((r = read(f, &c, sizeof(int))) != 0 && r != -1)
        list[i++] = c;

    for (i = 0; i < n; i++) {
        unsigned j = (i + 1) % n;
        while (j < n && list[j] > list[i]) {
            if (list[j] == list[i % n] + 1)
                map[(i % n)*n + j] = map[j*n + i % n] = 1;
            j++;
        }
    }

    int out_file;
    TRY((out_file = open(file_path, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0);

    TRY(write(out_file, &n, sizeof(unsigned)) < 0);
    TRY(write(out_file, map, n*n * sizeof(char)) < 0);

    close(out_file);
    close(f);
    free(s);
    free(map);
    free(list);
}

void generate_random_map(char *cmd, WINDOW *win) {
    UNUSED(win);

    unsigned n;
    char file_path[PATH_MAX];
    if (sscanf(cmd, "generate-random-map %u %s", &n, file_path) <= 0)
        return;

    char *new_file_path = expand_path(file_path);
    strcpy(file_path, new_file_path);
    free(new_file_path);

    if (n > 8 * sizeof(unsigned long long))
        return;

    char *tab;
    TRY((tab = calloc(n*n, sizeof(char))) == NULL);
    unsigned long long used_rooms_mask = 0;
    unsigned prev, curr, next;

    prev = -1;
    curr = rand() % n;
    while (used_rooms_mask != pow(2, n) - 1) {
        do {
            next = rand() % n;
        } while (next == curr || next == prev);
        tab[curr*n + next] = tab[next*n + curr] = 1;
        used_rooms_mask |= (1 << next);
        prev = curr;
        curr = next;
    }

    int f;
    TRY((f = open(file_path, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0);

    TRY(write(f, &n, sizeof(unsigned)) < 0);
    TRY(write(f, tab, n*n) < 0);

    free(tab);
    close(f);
}

void start_game(char *cmd, gameState_t *game, WINDOW *win) {
    SET_GAME_MODE(1);
    char path[PATH_MAX];
    if (sscanf(cmd, "start-game %s", path) <= 0)
        return;

    char *new_path = expand_path(path);
    strcpy(path, new_path);
    free(new_path);

    int f;
    TRY((f = open(path, O_RDONLY)) < 0);

    unsigned n;
    TRY(read(f, &n, sizeof(unsigned)) <= 0);

    pthread_mutex_lock(game->game_mutex);

    game->n = n;
    game->player_position = rand() % n;
    game->num_player_objects = 0;
    TRY((game->rooms_map = malloc(n*n * sizeof(char))) == NULL);
    TRY(read(f, game->rooms_map, n*n) <= 0);

    TRY((game->rooms = calloc(n, sizeof(room_t))) == NULL);

    for (unsigned i = 0; i < n; i++)
        game->rooms[i].id = i;

    unsigned num_objects_generated = 0;
    object_t *obj;
    while (num_objects_generated < floor(3 * n / 2)) {
        obj = malloc(sizeof(object_t));
        obj->id = num_objects_generated++;
        unsigned existing_room_id;
        unsigned assigned_room_id;
        do {
            existing_room_id = rand() % n;
        } while (game->rooms[existing_room_id].num_existing_objects == 2);
        do {
            assigned_room_id = rand() % n;
        } while (game->rooms[assigned_room_id].num_assigned_objects == 2);
        obj->assigned_room = assigned_room_id;
        game->rooms[existing_room_id].objects[game->rooms[existing_room_id].num_existing_objects++] = obj;
        game->rooms[assigned_room_id].num_assigned_objects++;
    }

    TRY(pthread_create(&game->auto_save_tid, NULL, auto_save_game, game));
    TRY(pthread_create(&game->alarm_generator_tid, NULL, alarm_generator, game));
    TRY(pthread_create(&game->swap_objects_tid, NULL, swap_objects, game));
    TRY(pthread_create(&game->user_signal_catcher_tid, NULL, user_signal_catcher, game));

    pthread_mutex_unlock(game->game_mutex);

    print_game(game, win);
}

void load_game(char *cmd, gameState_t *game, WINDOW *win) {
    SET_GAME_MODE(1);
    char path[PATH_MAX];
    if (sscanf(cmd, "load-game %s", path) <= 0)
        return;

    char *new_path = expand_path(path);
    strcpy(path, new_path);
    free(new_path);

    int f;
    TRY((f = open(path, O_RDONLY)) == -1);

    unsigned n;
    TRY(read(f, &n, sizeof(unsigned)) <= 0);

    pthread_mutex_lock(game->game_mutex);

    game->n = n;

    TRY((game->rooms_map = malloc(n*n * sizeof(char))) == NULL);
    TRY(read(f, game->rooms_map, n*n * sizeof(char)) <= 0);

    TRY(read(f, &game->player_position, sizeof(unsigned)) <= 0);
    TRY(read(f, &game->num_player_objects, sizeof(unsigned)) <= 0);
    TRY(game->num_player_objects > 0 && (game->player_objects[0] = malloc(sizeof(object_t))) == NULL);
    TRY(game->num_player_objects > 1 && (game->player_objects[1] = malloc(sizeof(object_t))) == NULL);
    for (unsigned i = 0; i < game->num_player_objects; i++)
        TRY(read(f, game->player_objects[i], sizeof(object_t)) <= 0);

    TRY((game->rooms = calloc(n, sizeof(room_t))) == NULL);
    for (unsigned i = 0; i < n; i++) {
        TRY(read(f, &game->rooms[i].id, sizeof(unsigned)) < 0);
        TRY(read(f, &game->rooms[i].num_existing_objects, sizeof(unsigned)) < 0);

        TRY(game->rooms[i].num_existing_objects > 0 && (game->rooms[i].objects[0] = calloc(1, sizeof(room_t))) == NULL);
        TRY(game->rooms[i].num_existing_objects > 1 && (game->rooms[i].objects[1] = calloc(1, sizeof(room_t))) == NULL);

        TRY(read(f, &game->rooms[i].num_assigned_objects, sizeof(unsigned)) < 0);
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++)
            TRY(read(f, game->rooms[i].objects[j], sizeof(object_t)) < 0);
    }

    pthread_mutex_unlock(game->game_mutex);

    TRY(pthread_create(&game->auto_save_tid, NULL, auto_save_game, game));
    TRY(pthread_create(&game->alarm_generator_tid, NULL, alarm_generator, game));
    TRY(pthread_create(&game->swap_objects_tid, NULL, swap_objects, game));
    TRY(pthread_create(&game->user_signal_catcher_tid, NULL, user_signal_catcher, game));

    print_game(game, win);

    close(f);
}
