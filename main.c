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
    mvwprintw(cmdWin, 0, 2, "Mode: Menu");

    wmove(cmdWin, 1, 2);
    wprintw(cmdWin, "Command: ");
    wrefresh(mainWin);
    wrefresh(cmdWin);

    char buf[maxX];
    int isEnd = 0;
    while (!isEnd) {
        wmove(cmdWin, 1, 11);
        whline(cmdWin, ' ', maxX - 12);
        wscanw(cmdWin, "%s", buf);
        command_t *cmd = parse_command(buf);
        if (cmd == NULL)
            continue;
        switch (cmd->type) {
            case Exit:
                isEnd = 1;
                break;
            default:
                break;
        }
        free(cmd);
        wrefresh(cmdWin);
    }

    delwin(mainWin);
    delwin(cmdWin);
    endwin();

    return EXIT_SUCCESS;
}
