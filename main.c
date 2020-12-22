/**
 * Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów
 * uczenia się z przedmiotu SOP1 została wykonana przeze mnie samodzielnie.
 * Piotr Rogulski
 * 305867
 */

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
    WINDOW *main_win = newwin(maxY - 3, maxX, 0, 0);
    WINDOW *cmd_win = newwin(3, maxX, maxY - 3, 0);

    wborder(main_win, 0, 0, 0, 0, 0, 0, 0, 0);
    wborder(cmd_win, 0, 0, 0, 0, 0, 0, 0, 0);

    wmove(cmd_win, 1, 2);
    wprintw(cmd_win, "Command: ");
    wrefresh(main_win);
    wrefresh(cmd_win);

    // Set up signals
    sigset_t new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGALRM);
    sigaddset(&new_mask, SIGUSR1);
    TRY(pthread_sigmask(SIG_BLOCK, &new_mask, NULL));

    // Main program loop
    char buf[maxX];
    pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;
    gameState_t game;
    game.game_mutex = &game_mutex;
    game.win = main_win;
    game.swap_seed = rand();
    while (1) {
        char* is_game_mode = getenv("IS_GAME_MODE");
        if (is_game_mode == NULL || atoi(is_game_mode) == 0)
            mvwprintw(cmd_win, 0, 2, " Mode: Menu ");
        else
            mvwprintw(cmd_win, 0, 2, " Mode: Game ");
        wrefresh(cmd_win);
        wmove(cmd_win, 1, 11);
        whline(cmd_win, ' ', maxX - 12);
        TRY(memset(buf, 0, maxX) == NULL);
        wgetstr(cmd_win, buf);
        int ret = exec_command(buf, main_win, &game);
        if (ret == INVALID_CMD)
            continue;
        if (ret == EXIT_CMD)
            break;
        wrefresh(cmd_win);
    }

    // curses cleanup
    delwin(main_win);
    delwin(cmd_win);
    endwin();

    return EXIT_SUCCESS;
}
