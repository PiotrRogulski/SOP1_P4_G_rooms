#include <ftw.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "menu_funs.h"
#include "types.h"

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f) {
    UNUSED(s);
    UNUSED(f);

    if (type != FTW_D)
        return 0;

    int id = open("/tmp/dir_tree.txt", O_WRONLY | O_CREAT | O_APPEND, 0777);
    if (write(id, name, strlen(name)) < 0)
        ERROR("Couldn't write to file");
    if (write(id, "\n", 1) < 0)
        ERROR("Couldn't write to file");
    close(id);
    return 0;
}

void map_from_dir_tree(char* cmd, WINDOW *win) {
    char dirPath[strlen(cmd)];
    char filePath[strlen(cmd)];
    if (sscanf(cmd, "map-from-dir-tree %s %s", dirPath, filePath) <= 0)
        return;

    if (remove("/tmp/dir_tree.txt") && errno != ENOENT)
        ERROR("Couldn't remove existing file");
    if (nftw(dirPath, walk_print, MAXFD, FTW_PHYS) < 0)
        ERROR("Couldn't walk the directory tree");

    unsigned n = 0;
    int f = open("/tmp/dir_tree.txt", O_RDONLY);
    if (f < 0)
        ERROR("Couldn't open /tmp/dir_tree.txt");
    char c;
    int r;
    while ((r = read(f, &c, 1)) != 0 && r != -1)
        if (c == '\n')
            n++;

    char *map = calloc(n*n + 1, sizeof(char));

    struct stat *s = malloc(sizeof(struct stat));
    if (stat("/tmp/dir_tree.txt", s) != 0)
        ERROR("Couldn't get info about /tmp/dir_tree.txt");

    char tree[s->st_size + 1];

    lseek(f, 0, SEEK_SET);
    if (read(f, tree, s->st_size) <= 0)
        ERROR("Couldn't read tree");

    mvwprintw(win, 1, 1, "%s", tree);
    wrefresh(win);

    close(f);
    free(s);
    free(map);
}

void generate_random_map(char *cmd, WINDOW *win) {
    UNUSED(win);

    unsigned n;
    char filePath[strlen(cmd)];
    if (sscanf(cmd, "generate-random-map %u %s", &n, filePath) <= 0)
        return;

    if (n > 8 * sizeof(unsigned long long))
        return;

    char *tab = calloc(n*n, sizeof(char));
    if (tab == NULL)
        ERROR("Couldn't allocate memory for the graph");
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

    int fileDes;
    if ((fileDes = open(filePath, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0)
        ERROR("Couldn't open file for writing graph");

    if (write(fileDes, &n, sizeof(unsigned)) < 0)
        ERROR("Couldn't write n to file");
    if (write(fileDes, tab, n*n) < 0)
        ERROR("Couldn't write graph to file");

    free(tab);
    close(fileDes);
}

void start_game(char *cmd, gameState_t *game, WINDOW *win) {
    SET_GAME_MODE(1);
    char path[strlen(cmd)];
    if (sscanf(cmd, "start-game %s", path) <= 0)
        return;

    int fileDes;
    if ((fileDes = open(path, O_RDONLY)) < 0)
        ERROR("Couldn't open map file");

    unsigned n;
    if (read(fileDes, &n, sizeof(unsigned)) <= 0)
        ERROR("Couldn't read n from map file");

    pthread_mutex_lock(game->game_mutex);

    game->n = n;
    game->player_position = rand() % n;
    game->num_player_objects = 0;
    if ((game->rooms_map = malloc(n*n * sizeof(char))) == NULL)
        ERROR("Couldn't allocate memory");
    if (read(fileDes, game->rooms_map, n*n) <= 0)
        ERROR("Couldn't read room map");

    if ((game->rooms = calloc(n, sizeof(room_t))) == NULL)
        ERROR("Couldn't allocate rooms");

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

    if (pthread_create(&game->auto_save_tid, NULL, auto_save_game, game))
        ERROR("Couldn't create auto-save thread");
    if (pthread_create(&game->alarm_generator_tid, NULL, alarm_generator, game))
        ERROR("Couldn't create alarm thread");

    pthread_mutex_unlock(game->game_mutex);

    print_game(game, win);
}

void load_game(char *cmd, gameState_t *game, WINDOW *win) {
    SET_GAME_MODE(1);
    char path[strlen(cmd)];
    if (sscanf(cmd, "load-game %s", path) <= 0)
        return;

    int f = open(path, O_RDONLY);
    if (f == -1)
        ERROR("Couldn't open file for reading");

    unsigned n;
    if (read(f, &n, sizeof(unsigned)) <= 0)
        ERROR("Couldn't read n from file");

    pthread_mutex_lock(game->game_mutex);

    game->n = n;

    if ((game->rooms_map = malloc(n*n * sizeof(char))) == NULL)
        ERROR("Couldn't allocate memory");
    if (read(f, game->rooms_map, n*n * sizeof(char)) <= 0)
        ERROR("Couldn't read map from file");

    if (read(f, &game->player_position, sizeof(unsigned)) <= 0)
        ERROR("Couldn't read player position");
    if (read(f, &game->num_player_objects, sizeof(unsigned)) <= 0)
        ERROR("Couldn't read inventory object count");
    if (game->num_player_objects > 0 && (game->player_objects[0] = malloc(sizeof(object_t))) == NULL)
        ERROR("Couldn't allocate player inventory slot 0");
    if (game->num_player_objects > 1 && (game->player_objects[1] = malloc(sizeof(object_t))) == NULL)
        ERROR("Couldn't allocate player inventory slot 1");
    for (unsigned i = 0; i < game->num_player_objects; i++)
        if (read(f, game->player_objects[i], sizeof(object_t)) <= 0)
            ERROR("Couldn't read object");

    if ((game->rooms = calloc(n, sizeof(room_t))) == NULL)
        ERROR("Couldn't allocate rooms");
    for (unsigned i = 0; i < n; i++) {
        if (read(f, &game->rooms[i].id, sizeof(unsigned)) < 0)
            ERROR("Couldn't read room id");
        if (read(f, &game->rooms[i].num_existing_objects, sizeof(unsigned)) < 0)
            ERROR("Couldn't read object count");

        if (game->rooms[i].num_existing_objects > 0 && (game->rooms[i].objects[0] = calloc(1, sizeof(room_t))) == NULL)
            ERROR("Couldn't allocate room object 0");
        if (game->rooms[i].num_existing_objects > 1 && (game->rooms[i].objects[1] = calloc(1, sizeof(room_t))) == NULL)
            ERROR("Couldn't allocate room object 1");

        if (read(f, &game->rooms[i].num_assigned_objects, sizeof(unsigned)) < 0)
            ERROR("Couldn't read assigned object count");
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++)
            if (read(f, game->rooms[i].objects[j], sizeof(object_t)) < 0)
                ERROR("Couldn't read object");
    }

    pthread_mutex_unlock(game->game_mutex);

    if (pthread_create(&game->auto_save_tid, NULL, auto_save_game, game))
        ERROR("Couldn't create auto-save thread");
    if (pthread_create(&game->alarm_generator_tid, NULL, alarm_generator, game))
        ERROR("Couldn't create alarm thread");

    print_game(game, win);

    close(f);
}
