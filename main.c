#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <linux/limits.h>

#include "util_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))

int main(int argc, char **argv) {
    srand(time(NULL));
    parse_args(argc, argv);

    initscr();
    echo();
    keypad(stdscr, TRUE);

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    WINDOW *mainWin = newwin(maxY - 3, maxX, 0, 0);
    WINDOW *cmdWin = newwin(3, maxX, maxY - 3, 0);

    wborder(mainWin, 0, 0, 0, 0, 0, 0, 0, 0);
    wborder(cmdWin, 0, 0, 0, 0, 0, 0, 0, 0);

    wmove(cmdWin, 1, 2);
    wprintw(cmdWin, "Command: ");
    wrefresh(mainWin);
    wrefresh(cmdWin);

    char buf[maxX];
    int isEnd = 0;
    gameState_t game;
    while (!isEnd) {
        char* isGameMode = getenv("IS_GAME_MODE");
        if (isGameMode == NULL || atoi(isGameMode) == 0)
            mvwprintw(cmdWin, 0, 2, " Mode: Menu ");
        else
            mvwprintw(cmdWin, 0, 2, " Mode: Game ");
        wrefresh(cmdWin);
        wmove(cmdWin, 1, 11);
        whline(cmdWin, ' ', maxX - 12);
        if (memset(buf, 0, maxX) == NULL)
            ERROR("Couldn't set buffer");
        wgetstr(cmdWin, buf);
        int ret = exec_command(buf, mainWin, &game);
        if (ret == INVALID_CMD)
            continue;
        switch (ret) {
            case EXIT_CMD:
                isEnd = 1;
                break;
            default:
                break;
        }
        wrefresh(cmdWin);
    }

    delwin(mainWin);
    delwin(cmdWin);
    endwin();

    return EXIT_SUCCESS;
}
