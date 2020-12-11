#include <ftw.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "game_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))

#define MAXFD 20

void map_from_dir_tree(char* cmd, WINDOW *win) {
    // TODO: implement
}

void generate_random_map(char *cmd, WINDOW *win) {
    int n;
    char filePath[strlen(cmd)];
    if (sscanf(cmd, "generate-random-map %d %s", &n, filePath) <= 0)
        return;

    wrefresh(win);

    char *tab = calloc(n*n + 1, sizeof(char));
    tab[n*n] = '\n';
    unsigned long long used_rooms_mask = 0; // max 64 rooms :(
    int curr, next;

    srand(time(NULL));
    curr = rand() % n;
    while (used_rooms_mask != (unsigned long long) pow(2, n) - 1) {
        do {
            next = rand() % n;
        } while (next == curr);
        tab[curr*n + next] = tab[next*n + curr] = 1;
        used_rooms_mask |= (1 << next);
        curr = next;
    }

    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            mvwprintw(win, i, 2*j, "%d ", tab[(i-1)*n + j-1]);
    wrefresh(win);

    int fileDes;
    if ((fileDes = open(filePath, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0)
        ERROR("Couldn't open file for writing graph");
    char *nBuf;
    int c;
    if ((c = asprintf(&nBuf, "%d\n", n)) < 0)
        ERROR("Couldn't convert n to a string");
    if (write(fileDes, nBuf, c) < 0)
        ERROR("Couldn't write n to file");
    if (write(fileDes, tab, n*n + 1) < 0)
        ERROR("Couldn't write graph to file");

    free(nBuf);
    free(tab);
    close(fileDes);
}

void start_game(void) {
    SET_GAME_MODE(1);
    // TODO: implement
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

