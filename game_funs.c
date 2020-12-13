#include <ftw.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "game_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))
#define UNUSED(x) (void)(x)
#define MAXFD 20

int walk_print(const char *name, const struct stat *s, int type, struct FTW *f) {
    UNUSED(s);
    UNUSED(f);

    if (type != FTW_D)
        return 0;

    int id = open("/tmp/dir_tree.txt", O_WRONLY | O_CREAT | O_APPEND, 0777);
    write(id, name, strlen(name));
    write(id, "\n", 1);
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

    int n = 0;
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
    read(f, tree, s->st_size);

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

    char *tab = calloc(n*n + 1, sizeof(char));
    if (tab == NULL)
        ERROR("Couldn't allocate memory for the graph");
    tab[n*n] = '\n';
    unsigned long long used_rooms_mask = 0;
    int prev, curr, next;

    prev = -1;
    curr = rand() % n;
    while (used_rooms_mask != pow(2, n) - 1) {
        do {
            next = rand() % n;
        } while (next == curr || next == prev);
        tab[curr*n + next] = tab[next*n + curr] = 48 + 1;
        used_rooms_mask |= (1 << next);
        prev = curr;
        curr = next;
    }

    int fileDes;
    if ((fileDes = open(filePath, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0)
        ERROR("Couldn't open file for writing graph");
    char *nBuf;
    int c;
    if ((c = asprintf(&nBuf, "%2d\n", n)) < 0)
        ERROR("Couldn't convert n to a string");
    if (write(fileDes, nBuf, c) < 0)
        ERROR("Couldn't write n to file");
    if (write(fileDes, tab, n*n + 1) < 0)
        ERROR("Couldn't write graph to file");

    free(nBuf);
    free(tab);
    close(fileDes);
}

void start_game(char *cmd, WINDOW *win, gameState_t *game) {
    UNUSED(win);

    SET_GAME_MODE(1);
    char path[strlen(cmd)];
    if (sscanf(cmd, "start-game %s", path) <= 0)
        return;

    int fileDes;
    if ((fileDes = open(path, O_RDONLY)) < 0)
        ERROR("Couldn't open map file");

    char nBuf[3];
    if (read(fileDes, nBuf, sizeof(nBuf)) <= 0)
        ERROR("Couldn't read n from map file");
    int n = atoi(nBuf);

    game->n = n;
    game->playerPosition = rand() % n;
    if ((game->roomsMap = malloc((n*n + 1) * sizeof(char))) == NULL)
        ERROR("Couldn't allocate memory");
    if (read(fileDes, game->roomsMap, n*n + 1) <= 0)
        ERROR("Couldn't read room map");

    if ((game->rooms = calloc(n, sizeof(room_t))) == NULL)
        ERROR("Couldn't allocate rooms");

    for (int i = 0; i < n; i++) {
        game->rooms[i].id = i;
        game->rooms[i].num_objects = 0;
    }
}

void load_game(void) {
    SET_GAME_MODE(1);
    // TODO: implement
}



void move_to(void) {
    // TODO: implement
}

void pick_up(void) {
    // TODO: implement
}

void drop(void) {
    // TODO: implement
}

void save(void) {
    // TODO: implement
}

void find_path(void) {
    // TODO: implement
}

void quit(gameState_t *game) {
    SET_GAME_MODE(0);

    free(game->rooms);
    free(game->roomsMap);
    // TODO: implement
}

