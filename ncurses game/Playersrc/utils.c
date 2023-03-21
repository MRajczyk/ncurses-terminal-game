#include "utils.h"

extern struct player_data* ReceivedData;
extern struct conn_players* ConnectedPlayers;

extern pthread_t board_handler, kb_handler;

void set_terminal()
{
    system("resize -s 28 111");
    srand(time(NULL));
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    //timeout(ROUND_MS);
    keypad(stdscr, TRUE);

    start_color();

    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_GREEN, 0, 400, 0);
    init_color(COLOR_MAGENTA, 600, 0, 600);

    init_pair(CO_PLAYER, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(CO_WALL, COLOR_WHITE, COLOR_WHITE);
    init_pair(CO_BEAST, COLOR_RED, COLOR_BLACK);
    init_pair(CO_COIN, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CO_CAMPSITE, COLOR_YELLOW, COLOR_GREEN);
    init_pair(CO_DROPPED, COLOR_WHITE, COLOR_BLUE);
    init_pair(CO_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(CO_BUSH, COLOR_GREEN, COLOR_BLACK);

    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    //terminal background
    attron(COLOR_PAIR(CO_DEFAULT));
    for (int i = 0; i < max_y; ++i)
    {
        for (int j = 0; j < max_x; ++j)
            mvprintw(i, j, " ");
    }
    attroff(COLOR_PAIR(CO_DEFAULT));
    refresh();
}

void swap(char * a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap(str + start, str + end);
        ++start;
        --end;
    }
}

char* itoa_base_10(int number, char* buffer)
{
    if(number == 0)
    {
        *buffer = '0';
        *(buffer + 1) = '\0';
        return buffer;
    }
    int is_neg = 0;
    if(number < 0)
    {
        number *= -1;
        is_neg = 1;
    }
    *buffer = '\0';
    int i = 1;
    while(number != 0)
    {
        *(buffer + i) = (number % 10) + '0';
        number /= 10;
        ++i;
    }
    if(is_neg)
    {
        *(buffer + i) = '-';
        ++i;
    }

    reverse(buffer, i);

    return buffer;
}
