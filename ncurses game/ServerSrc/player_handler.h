#ifndef PLAYER_HANDLER
#define PLAYER_HANDLER

#include "server.h"

int manage_player_shm(enum shm_action todo, int pid);

void* player_conn(void* arg);

void* player_disconn(void* arg);

#endif
