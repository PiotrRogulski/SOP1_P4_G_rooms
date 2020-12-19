#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "util_funs.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))

#define TRY(expr) if (expr) ERROR(#expr)

int main(int argc, char **argv) {
    srand(time(NULL));
    parse_args(argc, argv);

    // Initialize curses
    initscr();
    start_color();
    echo();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_BLACK, COLOR_RED);
    init_pair(2, COLOR_BLACK, COLOR_GREEN);

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

    // Set up signals
    sigset_t old_mask, new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGALRM);
    sigaddset(&new_mask, SIGUSR1);
    TRY(pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask));

    // Main program loop
    char buf[maxX];
    pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;
    gameState_t game;
    game.game_mutex = &game_mutex;
    game.win = mainWin;
    game.swap_seed = rand();
    while (1) {
        char* isGameMode = getenv("IS_GAME_MODE");
        if (isGameMode == NULL || atoi(isGameMode) == 0)
            mvwprintw(cmdWin, 0, 2, " Mode: Menu ");
        else
            mvwprintw(cmdWin, 0, 2, " Mode: Game ");
        wrefresh(cmdWin);
        wmove(cmdWin, 1, 11);
        whline(cmdWin, ' ', maxX - 12);
        TRY(memset(buf, 0, maxX) == NULL);
        wgetstr(cmdWin, buf);
        int ret = exec_command(buf, mainWin, &game);
        if (ret == INVALID_CMD)
            continue;
        if (ret == EXIT_CMD)
            break;
        wrefresh(cmdWin);
    }

    // curses cleanup
    delwin(mainWin);
    delwin(cmdWin);
    endwin();

    return EXIT_SUCCESS;
}
