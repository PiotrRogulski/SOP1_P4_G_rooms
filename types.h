#pragma once

#include <pthread.h>
#include <ncurses.h>

typedef enum cmdType {
    Map_from_dir_tree,
    Generate_random_map,
    Start_game,
    Load_game,
    Exit,

    Move_to,
    Pick_up,
    Drop,
    Save,
    Find_path,
    Quit,
} cmdType_t;

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
    char *roomsMap;
    room_t *rooms;
    unsigned playerPosition;
    unsigned n;
} gameState_t;

typedef struct timespec timespec_t;
