#pragma once

#include <pthread.h>
#include <ncurses.h>

typedef struct object {
    unsigned id;
    unsigned assigned_room;
} object_t;

typedef struct room {
    unsigned id;
    object_t *objects[2];
    unsigned num_existing_objects;
    unsigned num_assigned_objects;
} room_t;

typedef struct gameState {
    char *rooms_map;
    room_t *rooms;
    unsigned player_position;
    object_t *player_objects[2];
    unsigned num_player_objects;
    unsigned n;
} gameState_t;

typedef struct timespec timespec_t;

typedef struct pathFind {
    gameState_t *game;
    unsigned seed;
    unsigned *path;
    unsigned destination;
    unsigned length;
} pathFind_t;
