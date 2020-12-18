#pragma once

#include <linux/limits.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include "game_funs.h"
#include "menu_funs.h"
#include "types.h"

#define INVALID_CMD -1
#define OK_CMD       0
#define EXIT_CMD     1

/**
 * Parse commandline options
 */
void parse_args(int argc, char **argv);

/**
 * Set the path to the backup file in the environment
 */
void set_backup(char *path);

/**
 * Parse the command and run the selected function
 *
 * @param cmd  game command with arguments
 * @param win  main window woth game state output
 * @param game state of the current game together with info about threads
 */
int exec_command(char *cmd, WINDOW *win, gameState_t *game);

/**
 * Sends a SIGALRM to the autosave thread every 60 seconds
 */
void *alarm_generator(void *voidArgs);

void *user_signal_catcher(void *voidArgs);

/**
 * Runs in a seperate thread and autosaves the game upon every SIGALRM
 */
void *auto_save_game(void *voidArgs);

/**
 * Swaps two random objects in the game upon every SIGUSR1
 */
void *swap_objects(void *voidArgs);

/**
 * Thread cleanup handler for auto-save thread
 */
void unlock_mutexes(void *voidArgs);
