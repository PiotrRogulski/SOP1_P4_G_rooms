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
    }

    int num_objects_generated = 0;
    object_t *obj;
    while (num_objects_generated < floor(3 * n / 2)) {
        obj = malloc(sizeof(object_t));
        obj->id = num_objects_generated++;
        int existing_room_id;
        int assigned_room_id;
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

    print_table(game->roomsMap, n, win);
    print_curr_room(game->playerPosition, n, win);
    print_objects(game, win);
    wrefresh(win);
}

void load_game(void) {
    SET_GAME_MODE(1);
    // TODO: implement
}



void move_to(char *cmd, WINDOW *win, gameState_t *game) {
    unsigned x;
    unsigned curr = game->playerPosition;
    unsigned n = game->n;
    if (sscanf(cmd, "move-to %u", &x) <= 0)
        return;

    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

    if (x < n && game->roomsMap[x * n + curr] == '1') {
        game->playerPosition = x;
        mvwprintw(win, getmaxy(win) - 1, getmaxx(win) / 2 - 9, " Moved to room %u ", x);
        print_curr_room(x, n, win);
    } else if (x >= n) {
        mvwprintw(win, getmaxy(win) - 1, getmaxx(win) / 2 - 12, " Room %u doesn't exist ", x);
    } else {
        mvwprintw(win, getmaxy(win) - 1, getmaxx(win) / 2 - 12, " Cannot move to room %u ", x);

    }

    wrefresh(win);
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

void quit(gameState_t *game, WINDOW *win) {
    SET_GAME_MODE(0);

    for (unsigned i = 0; i < game->n; i++) {
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++) {
            free(game->rooms[i].objects[j]);
        }
    }


    free(game->rooms);
    free(game->roomsMap);

    werase(win);
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
    wrefresh(win);
    // TODO: implement
}



void print_table(char *table, unsigned n, WINDOW *win) {
    for (unsigned i = 0; i < n; i++) {
        mvwprintw(win, 3, 7 + 3*i, "%-2u", i);
        mvwprintw(win, 7 + 2*i, 2, "%2u", i);
    }

    mvwhline(win, 5, 2, ACS_HLINE, 5 + 3*n);
    mvwvline(win, 2, 5, ACS_VLINE, 5 + 2*n);
    mvwaddch(win, 5, 5, ACS_PLUS);

    for (unsigned i = 0; i < n; i++) {
        for (unsigned j = 0; j < n; j++) {
            if (table[i * n + j] == '1')
                wattron(win, COLOR_PAIR(2));
            else
                wattron(win, COLOR_PAIR(1));
            mvwaddch(win, 7 + 2*i, 7 + 3*j, ACS_BLOCK);
        }
    }
    wattroff(win, A_BOLD | COLOR_PAIR(1) | COLOR_PAIR(2));
}

void print_curr_room(unsigned curr, unsigned n, WINDOW *win) {
    mvwprintw(win, 9 + 2*n, 17, "  ");
    mvwprintw(win, 9 + 2*n, 3, "Current room: %u", curr);
}

void print_objects(gameState_t *game, WINDOW *win) {
    unsigned n = game->n;
    unsigned lines_printed = 0;

    mvwprintw(win, 3, 12 + 3*n, "Objects in the game: ");
    for (unsigned i = 0; i < n; i++) {
        if (game->rooms[i].num_existing_objects == 0)
            continue;
        mvwprintw(win, 5 + lines_printed, 14 + 3*n, "%2u:", i);
        for (unsigned j = 0; j < game->rooms[i].num_existing_objects; j++) {
            mvwprintw(win, 5 + lines_printed, 20 + 3*n, "%-2u assigned to %u", game->rooms[i].objects[j]->id, game->rooms[i].objects[j]->assigned_room);
            lines_printed++;
        }
        lines_printed++;
    }
}
