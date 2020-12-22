/**
 * Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów
 * uczenia się z przedmiotu SOP1 została wykonana przeze mnie samodzielnie.
 * Piotr Rogulski
 * 305867
 */

#pragma once

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <wordexp.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include "game_funs.h"
#include "menu_funs.h"
#include "types.h"

#define ERROR(source) (perror(source),\
                       fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
                       exit(EXIT_FAILURE))

#define TRY(expr) if (expr) ERROR(#expr)

#define INVALID_CMD -1
#define OK_CMD       0
#define EXIT_CMD     1

/**
 * Parse command line options.
 */
void parse_args(int argc, char **argv);

/**
 * Set the path to the backup file in the environment variable GAME_AUTOSAVE.
 */
void set_backup(char *path);

/**
 * Parse the command and run the selected function.
 */
int exec_command(char *cmd, WINDOW *win, gameState_t *game);

/**
 * Send a SIGALRM to the autosave thread every 60 seconds.
 */
void *alarm_generator(void *voidArgs);

/**
 * Catch SIGUSR1 and resend it to the swap thread.
 */
void *user_signal_catcher(void *voidArgs);

/**
 * Run a seperate thread and autosave the game upon every SIGALRM.
 */
void *auto_save_game(void *voidArgs);

/**
 * Swap two random objects in the game upon every SIGUSR1.
 */
void *swap_objects(void *voidArgs);

/**
 * Thread cleanup handler for autosave thread.
 */
void unlock_mutexes(void *voidArgs);

/**
 * Expand the path like a shell would.
 */
char *expand_path(char *path);
