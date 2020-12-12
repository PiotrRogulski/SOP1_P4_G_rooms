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
    UNUSED(type);
    UNUSED(f);

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
}

void generate_random_map(char *cmd, WINDOW *win) {
    unsigned n;
    char filePath[strlen(cmd)];
    if (sscanf(cmd, "generate-random-map %u %s", &n, filePath) <= 0)
        return;

    if (n > 8 * sizeof(unsigned long long))
        return;

    char *tab = malloc((n*n + 1) * sizeof(char));
    if (tab == NULL)
        ERROR("Couldn't allocate memory for the graph");
    if (memset(tab, 48, n*n) == NULL)
        ERROR("Couldn't set matrix to zeros");
    tab[n*n] = '\n';
    unsigned long long used_rooms_mask = 0;
    int prev, curr, next;

    srand(time(NULL));
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
    if ((game->roomsMap = malloc((n*n + 1) * sizeof(char))) == NULL)
        ERROR("Couldn't allocate memory");
    if (read(fileDes, game->roomsMap, n*n + 1) <= 0)
        ERROR("Couldn't read room map");

    if ((game->rooms = malloc(n * sizeof(room_t))) == NULL)
        ERROR("Couldn't allocate rooms");

    for (int i = 0; i < n; i++) {
        game->rooms[i].id = i;
        if (game->rooms[i].objects = malloc(2 * sizeof(object_t)))
            ERROR("Couldn't allocate room objects");
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

void quit(void) {
    SET_GAME_MODE(0);
    // TODO: implement
}

