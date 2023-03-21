#include "utils.h"

extern struct game_data gameData;
extern char board[HEIGHT][WIDTH];
extern int drops[HEIGHT][WIDTH];
extern sem_t sem_game_end;
extern int additional_treasure;
extern int beasts_alive;

void swap(char* a, char* b)
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

void set_terminal()
{
    system("resize -s 28 111");  //automatically set terminal size to perfectly fit the game screen
    srand(time(NULL));
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

int set_game()
{
    system("rm -rf /dev/shm/players");
    system("rm -rf /dev/shm/player_*");
    set_terminal();

    if(load_board("board.txt"))
    {
        puts("load_board");
        return -1;
    }

    gameData.round_counter = 0;

    int fd = shm_open("players", O_EXCL | O_CREAT | O_RDWR, 0666);
    if(fd == -1)
    {
        puts("Failed to create shm for connected players");
        return -1;
    }

    if(ftruncate(fd, sizeof(struct conn_players)) == -1)
    {
        shm_unlink("players");
        return -1;
    }

    gameData.players_info = mmap(NULL, sizeof(struct conn_players), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if(*(int*)gameData.players_info == -1)
    {
        shm_unlink("players");
        puts("players");
        return -1;
    }
    pthread_mutex_init(&gameData.game_mutex, NULL);

    for(int i = 0; i < PLAYERS; ++i)
    {
        gameData.players_shared[i] = NULL;

        gameData.players_info->player[i].action = empty;
        gameData.players_info->player[i].type = CPU;
        gameData.players_info->player[i].PID = -1;
    }

    sem_init(&gameData.players_info->ask, 1, 1);
    sem_init(&gameData.players_info->leave, 1, 0);
    sem_init(&gameData.players_info->waiting, 1, 0);
    sem_init(&gameData.players_info->exit, 1, 0);

    gameData.PID = getpid();
    find_free_spot('A', &gameData.campsite, 1);

    for(int i = 0; i < BEASTS; ++i)
    {
        gameData.beasts[i].point.x = -1;
        gameData.beasts[i].point.y = -1;
    }
    gameData.finish = 0;

    return 0;
}

int load_board(char* filename)
{
    if (!filename) return 1;

    FILE *fp = fopen(filename, "r");
    if (!fp)
        return 2;

    for(int i = 0; !feof(fp); ++i)
        fgets(board[i], WIDTH + 2, fp); // + 2, because fgets finishes reading upon reaching n - 1 chars if not reaching a newline, so the newline would be read in the next iteration, therefore generating MESS!

    fclose(fp);

    return 0;
}

void find_free_spot(char to_write, struct point* point, int write_on_board)
{
    while(1)
    {
        point->x = rand() % WIDTH - 1;
        if(point->x <= 0)
            continue;
        point->y = rand() % HEIGHT - 1;
        if(point->y <= 0)
            continue;

        for(int i = 0; i < PLAYERS; ++i)
        {
            if(gameData.players_info->player->action == playing && gameData.players_shared[i])
            {
                if(point->x == gameData.players_local[i].point.x && point->y == gameData.players_local[i].point.y)
                    continue;
            }
        }

        if(board[point->y][point->x] == ' ')
        {
            if(write_on_board)
                board[point->y][point->x] = to_write;
            return;
        }
    }
}
