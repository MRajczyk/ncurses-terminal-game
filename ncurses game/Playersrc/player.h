#ifndef PLAYER_H
#define PLAYER_H

#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <locale.h>

#define quit(c) ((c) == 'q' || (c) == 'Q')
#define treasure(c) ((c) == 'c' || (c) == 't' || (c) == 'T')

// for colors
#define CO_PLAYER 1
#define CO_WALL 2
#define CO_BEAST 3
#define CO_COIN 4
#define CO_CAMPSITE 5
#define CO_DROPPED 6
#define CO_DEFAULT 7
#define CO_BUSH 8

#define PLAYERS 4
#define HEIGHT 25
#define WIDTH 51

#define ROUND_MS 250

enum direction
{
    SIT = 0,
    UP,
    RIGHT,
    DOWN,
    LEFT
};

struct point
{
    int x;
    int y;
};

enum player_type
{
    CPU = 0,
    HUMAN
};

enum player_state
{
    empty = 0,
    waiting,
    playing,
    exitting
};

enum shm_action
{
    shm_delete = 0,
    open_shm
};

struct conn_players
{
    struct player_info
    {
        enum player_state action;
        enum player_type type;
        int PID;
    } player[PLAYERS];

    sem_t leave;
    sem_t ask;
    sem_t exit;
    sem_t waiting;
};

struct player_data
{
    int ID;
    int PID;
    int server_PID;
    enum player_type type;
    struct point point;
    struct point spawn_coords;
    int round_counter;
    enum direction direction;
    bool campsite_discovered;
    struct point campsite_coords;
    int slowed_down;
    int deaths;
    int coins_carried;
    int coins_brought;
    char player_board[HEIGHT][WIDTH];
    sem_t player_moved;
    sem_t player_continue;
    sem_t access_data;
};

void* print_things(void* arg);
void* kb_monitor(void* arg);
void game();

#include "utils.h"
#include "connection_man.h"

#endif
