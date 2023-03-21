#ifndef UTILS_H
#define UTILS_H

#include "server.h"

void set_terminal();

void swap(char* a, char* b);

void reverse(char str[], int length);

char* itoa_base_10(int number, char* buffer);

int set_game();

int load_board(char* filename);

void find_free_spot(char to_write, struct point* point, int write_on_board);

#endif
