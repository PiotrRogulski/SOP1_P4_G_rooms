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
 * Set the path to the backup file in the environment.
 */
void set_backup(char *path);

/**
 * Parse the command and run the selected function.
 */
int exec_command(char *cmd, WINDOW *win, gameState_t *game);

/**
 * Sends a SIGALRM to the autosave thread every 60 seconds.
 */
void *alarm_generator(void *voidArgs);

/**
 * Catches SIGUSR1 and resends it to the swap thread.
 */
void *user_signal_catcher(void *voidArgs);

/**
 * Runs in a seperate thread and autosaves the game upon every SIGALRM.
 */
void *auto_save_game(void *voidArgs);

/**
 * Swaps two random objects in the game upon every SIGUSR1.
 */
void *swap_objects(void *voidArgs);

/**
 * Thread cleanup handler for auto-save thread.
 */
void unlock_mutexes(void *voidArgs);

/**
 * Expand the path like a shell would.
 */
char *expand_path(char *path);
