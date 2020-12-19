#pragma once

#include <ncurses.h>
#include <pthread.h>
#include <signal.h>

/**
 * Holds information about an object
 */
typedef struct object {
    unsigned id;
    unsigned assigned_room;
} object_t;

/**
 * Holds information about a room (this includes objects in the room)
 */
typedef struct room {
    unsigned id;
    object_t *objects[2];
    unsigned num_existing_objects;
    unsigned num_assigned_objects;
} room_t;

/**
 * Holds information about the state of the game and necessary thread info: thread IDs and mutexes
 */
typedef struct gameState {
    char *rooms_map;
    room_t *rooms;
    unsigned player_position;
    object_t *player_objects[2];
    unsigned num_player_objects;
    unsigned n;
    pthread_mutex_t *game_mutex;
    pthread_t auto_save_tid;
    pthread_t alarm_generator_tid;
    pthread_t user_signal_catcher_tid;
    pthread_t swap_objects_tid;
    WINDOW *win;
    unsigned swap_seed;
} gameState_t;

typedef struct timespec timespec_t;

/**
 * Data type for path finding worker threads
 */
typedef struct pathFind {
    gameState_t *game;
    unsigned seed;
    unsigned *path;
    unsigned destination;
    unsigned length;
} pathFind_t;
