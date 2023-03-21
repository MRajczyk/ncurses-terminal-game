#include "player.h"

struct player_data* ReceivedData;
struct conn_players* ConnectedPlayers;

pthread_t board_handler, kb_handler;

void* print_things(void* arg)
{
    WINDOW* board_win = newwin(HEIGHT + 2, WIDTH + 2, 0, 0);
    WINDOW* sinfo = newwin(5, 56, 0, WIDTH + 4);
    WINDOW* pinfo = newwin(11, 56, 5, WIDTH + 4);
    WINDOW* legend = newwin(12, 56, 16, WIDTH + 4);

    while(1)
    {
        sem_wait(&ReceivedData->access_data);
        //board
        if(kill(ReceivedData->server_PID, 0) != 0)
        {
            exit_server(ReceivedData->PID);
            return NULL;
        }
        for(int i = 0; i < HEIGHT; ++i)
        {
            for(int j = 0; j < WIDTH; ++j)
            {
                if(treasure(ReceivedData->player_board[i][j]))
                    wattron(board_win, COLOR_PAIR(CO_COIN));
                else if(ReceivedData->player_board[i][j] == '|')
                    wattron(board_win, COLOR_PAIR(CO_WALL));
                else if(ReceivedData->player_board[i][j] == 'A')
                    wattron(board_win, COLOR_PAIR(CO_CAMPSITE));
                else if(ReceivedData->player_board[i][j] == 'D')
                    wattron(board_win, COLOR_PAIR(CO_DROPPED));
                else if(ReceivedData->player_board[i][j] == '.')
                {
                    ReceivedData->player_board[i][j] = ' ';
                    wattron(board_win, COLOR_PAIR(CO_DEFAULT));
                }
                else if(ReceivedData->player_board[i][j] == '#')
                    wattron(board_win, COLOR_PAIR(CO_BUSH));
                else if(ReceivedData->player_board[i][j] == '*')
                    wattron(board_win, COLOR_PAIR(CO_BEAST));
                else if(ReceivedData->player_board[i][j] >= '1' && ReceivedData->player_board[i][j] <= '4')
                    wattron(board_win, COLOR_PAIR(CO_PLAYER));
                else
                    wattron(board_win, COLOR_PAIR(CO_DEFAULT));
                mvwprintw(board_win, i + 1, j + 1, "%c", ReceivedData->player_board[i][j]);
            }
        }
        wattron(board_win, COLOR_PAIR(CO_DEFAULT));
        box(board_win, 0, 0);

        //server info
        wattron(sinfo, COLOR_PAIR(CO_DEFAULT));
        box(sinfo, 0, 0);
        mvwprintw(sinfo, 1, 2, "Server's PID - %d", ReceivedData->server_PID);
        mvwprintw(sinfo, 2, 2, "Round number - %d", ReceivedData->round_counter);
        if(ReceivedData->campsite_discovered == true)
            mvwprintw(sinfo, 3, 2, "Campsite coords - %02d/%02d", ReceivedData->campsite_coords.y, ReceivedData->campsite_coords.x);
        else
            mvwprintw(sinfo, 3, 2, "Campsite coords unknown");

        //players info
        for(int j = 0; j < 7; ++j)
            mvwprintw(pinfo, 3 + j, 14, "       ");
        wattron(pinfo, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(pinfo, 1, 2, "Player info:");
        mvwprintw(pinfo, 2, 2, "        Player %d", ReceivedData->ID);
        mvwprintw(pinfo, 3, 2, "PID");
        mvwprintw(pinfo, 4, 2, "Type");
        mvwprintw(pinfo, 5, 2, "Coords");
        mvwprintw(pinfo, 6, 2, "Deaths");
        mvwprintw(pinfo, 7, 2, "Coins");
        mvwprintw(pinfo, 8, 2, " Carried");
        mvwprintw(pinfo, 9, 2, " Brought");

        mvwprintw(pinfo, 3, 12, "%d", ReceivedData->PID);
        mvwprintw(pinfo, 4, 12, "HUMAN");
        mvwprintw(pinfo, 5, 12, "%02d/%02d", ReceivedData->point.y, ReceivedData->point.x);
        mvwprintw(pinfo, 6, 14, "%d", ReceivedData->deaths);
        mvwprintw(pinfo, 8, 14, "%d", ReceivedData->coins_carried);
        mvwprintw(pinfo, 9, 14, "%d", ReceivedData->coins_brought);

        sem_post(&ReceivedData->access_data);

        box(pinfo, 0, 0);

        //legend
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        box(legend, 0, 0);
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 1, 2, "Legend:");
        //wall
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 2, 2, "Wall - ");
        wattron(legend, COLOR_PAIR(CO_WALL));
        mvwprintw(legend, 2, 9, " ");
        //players
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 3, 2, "Player - ");
        wattron(legend, COLOR_PAIR(CO_PLAYER));
        mvwprintw(legend, 3, 11, "1234");
        //coin
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 4, 2, "Coin - ");
        wattron(legend, COLOR_PAIR(CO_COIN));
        mvwprintw(legend, 4, 9, "c");
        //small treasure
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 5, 2, "Small treasure - ");
        wattron(legend, COLOR_PAIR(CO_COIN));
        mvwprintw(legend, 5, 19, "t");
        //LARGE treasure
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 6, 2, "Large treasure - ");
        wattron(legend, COLOR_PAIR(CO_COIN));
        mvwprintw(legend, 6, 19, "T");
        //Campsite
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 7, 2, "Campsite - ");
        wattron(legend, COLOR_PAIR(CO_CAMPSITE));
        mvwprintw(legend, 7, 13, "A");
        //Dropped  treasure
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 8, 2, "Player drop - ");
        wattron(legend, COLOR_PAIR(CO_DROPPED));
        mvwprintw(legend, 8, 16, "D");
        //beast
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 9, 2, "Beast - ");
        wattron(legend, COLOR_PAIR(CO_BEAST));
        mvwprintw(legend, 9, 10, "*");
        //bush
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 10, 2, "Bush - ");
        wattron(legend, COLOR_PAIR(CO_BUSH));
        mvwprintw(legend, 10, 9, "#");
        //movement
        wattron(legend, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(legend, 1, 27, "Movement: ");
        mvwprintw(legend, 2, 27, "Move up - w/W/\u2191");
        mvwprintw(legend, 3, 27, "Move down - s/S/\u2193");
        mvwprintw(legend, 4, 27, "Move right - d/D/\u2192");
        mvwprintw(legend, 5, 27, "Move left - a/A/\u2190");

        //make everything visible
        wrefresh(board_win);
        wrefresh(pinfo);
        wrefresh(sinfo);
        wrefresh(legend);
    }
}

void* kb_monitor(void* arg)
{
    while(1)
    {
        sem_wait(&ReceivedData->player_continue);
        int c = getch();
	    if(c == ERR)
	        continue;
        flushinp();

        if(quit(c))
        {
            exit_server(ReceivedData->PID);
            return NULL;
        }
        else if(c == KEY_UP || c == 'w' || c == 'W')
            ReceivedData->direction = UP;
        else if(c == KEY_DOWN || c == 's' || c == 'S')
            ReceivedData->direction = DOWN;
        else if(c == KEY_RIGHT || c == 'd' || c == 'D')
            ReceivedData->direction = RIGHT;
        else if(c == KEY_LEFT || c == 'a' || c == 'A')
            ReceivedData->direction = LEFT;

        sem_post(&ReceivedData->player_moved);
    }
}

void game()
{
    int fd = shm_open("players", O_RDWR, 0666);

    if(fd == -1)
    {
        puts("Failed to open shm for connected players");
        return;
    }

    if(ftruncate(fd, sizeof(struct conn_players)) == -1)
        return;


    ConnectedPlayers = mmap(NULL, sizeof(struct conn_players), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if(*(int*)ConnectedPlayers == -1)
    {
        puts("MmapConnPlayers");
        return;
    }

    if(join_server() != 1)
    {
        munmap(ConnectedPlayers, sizeof(struct conn_players));
        return;
    }

    pthread_create(&board_handler, NULL, print_things, NULL);
    pthread_create(&kb_handler, NULL, kb_monitor, NULL);

    pthread_join(board_handler, NULL);
    pthread_join(kb_handler, NULL);
}
