#ifndef SERVER_H
#define SERVER_H

//#define _XOPEN_SOURCE_EXTENDED

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
#include <math.h>

#define quit(c) ((c) == 'q' || (c) == 'Q' || (c) == EOF)
#define beast(c) ((c) == 'b' || (c) == 'B')
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

#define P_SIGHT 5
#define PLAYERS 4
#define BEASTS 25

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
    shm_create
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

struct beast
{
    pthread_t beast_thread;
    sem_t beast_move;
    sem_t beast_moved;

    struct point point;
    enum direction direction;
    int slowed_down;
};

struct player_data_local
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

struct game_data
{
    struct conn_players* players_info;

    int PID;
    int round_counter;
    struct point campsite;

    struct player_data_local players_local[PLAYERS];
    struct player_data* players_shared[PLAYERS];
    struct beast beasts[BEASTS];

    pthread_mutex_t game_mutex;
    int finish;
};

void game();

void* print_things(void* arg);
void* kb_monitor(void* arg);

void next_round();

#include "beast.h"
#include "utils.h"
#include "player_handler.h"

#endif
