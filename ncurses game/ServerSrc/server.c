#include "server.h"

//#pragma clang diagnostic push
//#pragma ide diagnostic ignored "EndlessLoop"
struct game_data gameData;
char board[HEIGHT][WIDTH] = {0};
int drops[HEIGHT][WIDTH] = {0};
sem_t sem_game_end;
int additional_treasure = 40;
int beasts_alive = 0;

void* print_things(void* arg)
{
    WINDOW* board_win = newwin(HEIGHT + 2, WIDTH + 2, 0, 0);
    WINDOW* sinfo = newwin(5, 56, 0, WIDTH + 4);
    WINDOW* pinfo = newwin(11, 56, 5, WIDTH + 4);
    WINDOW* legend = newwin(12, 56, 16, WIDTH + 4);

    while(1)
    {
        pthread_mutex_lock(&gameData.game_mutex);
        if(gameData.finish == 1)
        {
            system("clear");
            pthread_mutex_unlock(&gameData.game_mutex);
            return NULL;
        }

        board[gameData.campsite.y][gameData.campsite.x] = 'A';

        //board
        for(int i = 0; i < HEIGHT; ++i)
        {
            for(int j = 0; j < WIDTH; ++j)
            {
                 if(treasure(board[i][j]))
                     wattron(board_win, COLOR_PAIR(CO_COIN));
                 else if(board[i][j] == '|')
                     wattron(board_win, COLOR_PAIR(CO_WALL));
                 else if(board[i][j] == 'A')
                     wattron(board_win, COLOR_PAIR(CO_CAMPSITE));
                 else if(board[i][j] == 'D')
                     wattron(board_win, COLOR_PAIR(CO_DROPPED));
                 else if(board[i][j] == '.')
                 {
                     board[i][j] = ' ';
                     wattron(board_win, COLOR_PAIR(CO_DEFAULT));
                 }
                 else if(board[i][j] == '#')
                     wattron(board_win, COLOR_PAIR(CO_BUSH));
                 else
                     wattron(board_win, COLOR_PAIR(CO_DEFAULT));
                 mvwprintw(board_win, i + 1, j + 1, "%c", board[i][j]);
            }
        }

        // players
        wattron(board_win,COLOR_PAIR(CO_PLAYER));
        for(int i = 0; i < PLAYERS; ++i)
        {
            if(gameData.players_shared[i] != NULL)
                mvwprintw(board_win, gameData.players_local[i].point.y + 1, gameData.players_local[i].point.x + 1, "%d", i + 1);
        }

        // beasts
        wattron(board_win, COLOR_PAIR(CO_BEAST));
        for(int i = 0; i < BEASTS; ++i)
        {
            if(gameData.beasts[i].point.x >= 0)
                mvwprintw(board_win, gameData.beasts[i].point.y + 1, gameData.beasts[i].point.x + 1, "*");
        }

        wattron(board_win, COLOR_PAIR(CO_DEFAULT));
        box(board_win, 0, 0);

        //server info
        wattron(sinfo, COLOR_PAIR(CO_DEFAULT));
        box(sinfo, 0, 0);
        mvwprintw(sinfo, 1, 2, "Server's PID - %d", gameData.PID);
        wattron(sinfo, COLOR_PAIR(CO_COIN));
        mvwprintw(sinfo, 1, 28, "c/t/T to be spawned - %02d", additional_treasure);
        wattron(sinfo, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(sinfo, 2, 2, "Round number - %d", gameData.round_counter);
        wattron(sinfo, COLOR_PAIR(CO_BEAST));
        mvwprintw(sinfo, 2, 28, "Beasts alive - %02d", beasts_alive);
        wattron(sinfo, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(sinfo, 3, 2, "Campsite position - %02d x %02d", gameData.campsite.y, gameData.campsite.x);

        //players info
        wattron(pinfo, COLOR_PAIR(CO_DEFAULT));
        mvwprintw(pinfo, 1, 2, "Player info:");
        mvwprintw(pinfo, 2, 2, "        Player 1    Player 2    Player 3    Player 4");
        mvwprintw(pinfo, 3, 2, "PID");
        mvwprintw(pinfo, 4, 2, "Type");
        mvwprintw(pinfo, 5, 2, "Coords");
        mvwprintw(pinfo, 6, 2, "Deaths");
        mvwprintw(pinfo, 7, 2, "Coins");
        mvwprintw(pinfo, 8, 2, " Carried");
        mvwprintw(pinfo, 9, 2, " Brought");
        for(int i = 0; i < PLAYERS; ++i)
        {
            for(int j = 0; j < 7; ++j)
                mvwprintw(pinfo, 3 + j, 11 + 11 * i, "         ");

            if(gameData.players_shared[i])
            {
                mvwprintw(pinfo, 3, 11 + 12 * i, "%d", gameData.players_local[i].PID);
                mvwprintw(pinfo, 4, 11 + 12 * i, "%s", gameData.players_local[i].type == CPU ? "CPU" : "HUMAN");
                mvwprintw(pinfo, 5, 11 + 12 * i, "%02d/%02d", gameData.players_local[i].point.y, gameData.players_local[i].point.x);
                mvwprintw(pinfo, 6, 13 + 12 * i, "%d", gameData.players_local[i].deaths);
                mvwprintw(pinfo, 8, 13 + 12 * i, "%d", gameData.players_local[i].coins_carried);
                mvwprintw(pinfo, 9, 13 + 12 * i, "%d", gameData.players_local[i].coins_brought);
            }
            else
            {
                mvwprintw(pinfo, 3, 12 + 12 * i, " -");
                mvwprintw(pinfo, 4, 12 + 12 * i, " -");
                mvwprintw(pinfo, 5, 12 + 12 * i, "-/-");
                mvwprintw(pinfo, 6, 12 + 12 * i, " -");
                mvwprintw(pinfo, 8, 12 + 12 * i, " -");
                mvwprintw(pinfo, 9, 12 + 12 * i, " -");
            }

        }
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

        wrefresh(board_win);
        wrefresh(pinfo);
        wrefresh(sinfo);
        wrefresh(legend);

        gameData.round_counter++;

        pthread_mutex_unlock(&gameData.game_mutex);
        next_round();
    }
}

void* kb_monitor(void* arg)
{
    while(1)
    {
        int c = getch();
	if(c == ERR)
            continue;
	flushinp();

        if(quit(c))
        {
            gameData.finish = 1;
            sem_post(&sem_game_end);
            return NULL;
        }
        else if(beast(c))
        {
            pthread_mutex_lock(&gameData.game_mutex);
            for(int i = 0; i < BEASTS; ++i)
            {
                if(gameData.beasts[i].point.x < 0)
                {
                    sem_init(&gameData.beasts[i].beast_move, 0, 0);
                    sem_init(&gameData.beasts[i].beast_moved, 0, 0);
                    find_free_spot('*', &gameData.beasts[i].point, 0);
                    gameData.beasts[i].slowed_down = 0;
                    pthread_create(&gameData.beasts[i].beast_thread, NULL, beast_mov, &i);
                    ++beasts_alive;
                    break;
                }
            }
            pthread_mutex_unlock(&gameData.game_mutex);
        }
        else if(treasure(c))
        {
            pthread_mutex_lock(&gameData.game_mutex);
            if(additional_treasure <= 0)
            {
                pthread_mutex_unlock(&gameData.game_mutex);
                continue;
            }
            struct point temp;
            find_free_spot(c, &temp, 1);
            board[temp.y][temp.x] = c;
            --additional_treasure;
            pthread_mutex_unlock(&gameData.game_mutex);
        }
    }
}

void game()
{
    pthread_t board_handler, player_conn_handler, player_disconn_handler, kb_handler;

    pthread_create(&board_handler, NULL, print_things, NULL);
    pthread_create(&player_conn_handler, NULL, player_conn, NULL);
    pthread_create(&player_disconn_handler, NULL, player_disconn, NULL);
    pthread_create(&kb_handler, NULL, kb_monitor, NULL);


    //semaphore signalling game end posted by kb_handler
    sem_init(&sem_game_end, 0, 0);
    sem_wait(&sem_game_end);

    sem_post(&gameData.players_info->waiting);
    sem_post(&gameData.players_info->exit);

    pthread_join(board_handler, NULL);
    pthread_join(player_conn_handler, NULL);
    pthread_join(player_disconn_handler, NULL);
    pthread_join(kb_handler, NULL);

    for(int i = 0; i < PLAYERS; ++i)
    {
        if(gameData.players_shared[i])
        {
            manage_player_shm(shm_delete, gameData.players_info->player[i].PID);
            kill(gameData.players_info->player[i].PID, SIGHUP);

            gameData.players_info->player[i].action = empty;
            gameData.players_info->player[i].type = CPU;
            gameData.players_info->player[i].PID = -1;

            sem_destroy(&gameData.players_shared[i]->player_moved);
            sem_destroy(&gameData.players_shared[i]->player_continue);

            munmap(gameData.players_shared[i], sizeof(struct player_data));
            gameData.players_shared[i] = NULL;
        }
    }

    for(int i = 0; i < BEASTS; ++i)
    {
        if(gameData.beasts[i].point.x >= 0)
        {
            pthread_cancel(gameData.beasts[i].beast_thread);
            sem_destroy(&gameData.beasts[i].beast_move);
            sem_destroy(&gameData.beasts[i].beast_moved);
        }
    }

    sem_destroy(&gameData.players_info->ask);
    sem_destroy(&gameData.players_info->leave);
    sem_destroy(&gameData.players_info->waiting);
    sem_destroy(&gameData.players_info->exit);
    munmap(gameData.players_info, sizeof(struct conn_players));
    shm_unlink("players");

    pthread_mutex_destroy(&gameData.game_mutex);
    sem_destroy(&sem_game_end);

    endwin();
    system("reset");
}

void next_round()
{
    pthread_mutex_lock(&gameData.game_mutex);
    int starting_x[4];
    int starting_y[4];

    for(int i = 0; i < PLAYERS; ++i)
    {
        if(gameData.players_shared[i])
            sem_post(&gameData.players_shared[i]->player_continue);
    }
    pthread_mutex_unlock(&gameData.game_mutex);
    usleep(ROUND_MS * 1000);
    pthread_mutex_lock(&gameData.game_mutex);
    for(int i = 0; i < PLAYERS; ++i)
    {
        if(gameData.players_shared[i])
        {
            starting_x[i] = gameData.players_local[i].point.x;
            starting_y[i] = gameData.players_local[i].point.y;
            if(kill(gameData.players_shared[i]->PID, 0) != 0) //if a player has disconected, we need to clean the remnants
            {
                //clean remnants of a player in the the players_info structure
                sem_destroy(&gameData.players_shared[i]->player_moved);
                sem_destroy(&gameData.players_shared[i]->player_continue);
                sem_destroy(&gameData.players_shared[i]->access_data);
                munmap(gameData.players_shared[i], sizeof(struct player_data));
                manage_player_shm(shm_delete, gameData.players_info->player[i].PID);
                gameData.players_shared[i] = NULL;
                gameData.players_info->player[i].action = empty;
                gameData.players_info->player[i].type = CPU;
                gameData.players_info->player[i].PID = -1;
                continue;
            }
            gameData.players_local[i].round_counter = gameData.round_counter;
        }
    }

    for(int i = 0; i < PLAYERS; ++i)
    {
        if(gameData.players_shared[i])
        {
            if(sem_trywait(&gameData.players_shared[i]->player_moved) == 0) //only if the player made a move change anything
            {
                if(gameData.players_local[i].slowed_down == 0)
                {
                    switch(gameData.players_shared[i]->direction)
                    {
                        case UP:
                            if(gameData.players_local[i].point.y - 1 >= 1 && board[gameData.players_local[i].point.y - 1][gameData.players_local[i].point.x] != '|')
                                gameData.players_local[i].point.y -= 1;
                            break;
                        case DOWN:
                            if(gameData.players_local[i].point.y + 1 < HEIGHT - 1 && board[gameData.players_local[i].point.y + 1][gameData.players_local[i].point.x] != '|')
                                gameData.players_local[i].point.y += 1;
                            break;
                        case RIGHT:
                            if(gameData.players_local[i].point.x + 1 < WIDTH - 1 && board[gameData.players_local[i].point.y][gameData.players_local[i].point.x + 1] != '|')
                                gameData.players_local[i].point.x += 1;
                            break;
                        case LEFT:
                            if(gameData.players_local[i].point.x - 1 >= 1 && board[gameData.players_local[i].point.y][gameData.players_local[i].point.x - 1] != '|')
                                gameData.players_local[i].point.x -= 1;
                            break;
                        default:
                        case SIT:
                            break;
                    }
                }
                switch(board[gameData.players_local[i].point.y][gameData.players_local[i].point.x]) //if player has moved, see what field he has landed on and if there is a need to update his statistics
                {
                    case 'c':
                        gameData.players_local[i].coins_carried += 1;
                        board[gameData.players_local[i].point.y][gameData.players_local[i].point.x] = ' ';
                        ++additional_treasure;
                        break;
                    case 't':
                        gameData.players_local[i].coins_carried += 10;
                        board[gameData.players_local[i].point.y][gameData.players_local[i].point.x] = ' ';
                        ++additional_treasure;
                        break;
                    case 'T':
                        gameData.players_local[i].coins_carried += 50;
                        board[gameData.players_local[i].point.y][gameData.players_local[i].point.x] = ' ';
                        ++additional_treasure;
                        break;
                    case 'D':
                        gameData.players_local[i].coins_carried += drops[gameData.players_local[i].point.y][gameData.players_local[i].point.x];
                        board[gameData.players_local[i].point.y][gameData.players_local[i].point.x] = ' ';
                        drops[gameData.players_local[i].point.y][gameData.players_local[i].point.x] = 0;
                        break;
                    case 'A':
                        gameData.players_local[i].coins_brought += gameData.players_local[i].coins_carried;
                        gameData.players_local[i].coins_carried = 0;
                        break;
                    case '#':
                        gameData.players_local[i].slowed_down = !gameData.players_local[i].slowed_down;
                        break;
                }
                gameData.players_local[i].direction = SIT;
            }
        }
    }


    int dead[PLAYERS] = {0};
    for(int i = 0; i < PLAYERS; ++i) //check for collisions with another players
    {
        for(int j = 0; j < PLAYERS; ++j)
        {
            if(gameData.players_shared[i] && gameData.players_shared[j] && i != j)
            {
                if(gameData.players_local[i].point.y == gameData.players_local[j].point.y && gameData.players_local[i].point.x == gameData.players_local[j].point.x)
                {
                    dead[i] = 1;
                    if(gameData.players_local[i].coins_carried)
                    {
                        board[gameData.players_local[i].point.y][gameData.players_local[i].point.x] = 'D';
                        drops[gameData.players_local[i].point.y][gameData.players_local[i].point.x] += gameData.players_local[i].coins_carried;
                    }
                }
            }
        }
    }

    for(int i = 0; i < BEASTS; ++i) //let beasts make a move
    {
        if(gameData.beasts[i].point.x >= 0)
        {
            sem_post(&gameData.beasts[i].beast_move);
            sem_wait(&gameData.beasts[i].beast_moved);
            if(gameData.beasts[i].slowed_down == 0)
            {
                switch(gameData.beasts[i].direction)
                {
                    case UP:
                        if(gameData.beasts[i].point.y - 1 >= 1 && board[gameData.beasts[i].point.y - 1][gameData.beasts[i].point.x] != '|')
                            gameData.beasts[i].point.y -= 1;
                        break;
                    case DOWN:
                        if(gameData.beasts[i].point.y + 1 < HEIGHT - 1 && board[gameData.beasts[i].point.y + 1][gameData.beasts[i].point.x] != '|')
                            gameData.beasts[i].point.y += 1;
                        break;
                    case RIGHT:
                        if(gameData.beasts[i].point.x + 1 < WIDTH - 1 && board[gameData.beasts[i].point.y][gameData.beasts[i].point.x + 1] != '|')
                            gameData.beasts[i].point.x += 1;
                        break;
                    case LEFT:
                        if(gameData.beasts[i].point.x - 1 >= 1 && board[gameData.beasts[i].point.y][gameData.beasts[i].point.x - 1] != '|')
                            gameData.beasts[i].point.x -= 1;
                        break;
                    default:
                    case SIT:
                        break;
                }
                if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x] == '#')
                    gameData.beasts[i].slowed_down = 1;
            }
            else
            {
                if(gameData.beasts[i].direction != SIT)
                    gameData.beasts[i].slowed_down = 0;
            }
        }
    }

    for(int i = 0; i < BEASTS; ++i) //check for collisions with beasts
    {
        if(gameData.beasts[i].point.x > 0)
        {
            for(int j = 0; j < PLAYERS; ++j)
            {
                if(gameData.players_shared[j])
                {
                    if(gameData.beasts[i].point.y == gameData.players_local[j].point.y && gameData.beasts[i].point.x == gameData.players_local[j].point.x)
                    {
                        dead[j] = 1;

                        if(gameData.players_local[j].coins_carried)
                        {
                            board[gameData.players_local[j].point.y][gameData.players_local[j].point.x] = 'D';
                            drops[gameData.players_local[j].point.y][gameData.players_local[j].point.x] += gameData.players_local[j].coins_carried;
                        }
                        pthread_cancel(gameData.beasts[i].beast_thread);
                        sem_destroy(&gameData.beasts[i].beast_moved);
                        sem_destroy(&gameData.beasts[i].beast_move);

                        gameData.beasts[i].point.x = -1;
                        gameData.beasts[i].point.y = -1;
                        gameData.beasts[i].direction = SIT;
                        --beasts_alive;
                        break;
                    }
                }
            }
        }
    }

    for(int i = 0; i < PLAYERS; ++i)  //if any players died, respawn them and clear their last position on the map before dying to avoid confusion
    {
        if(gameData.players_shared[i])
        {
            if(dead[i])
            {
                gameData.players_local[i].point.x = gameData.players_local[i].spawn_coords.x;
                gameData.players_local[i].point.y = gameData.players_local[i].spawn_coords.y;

                gameData.players_shared[i]->player_board[starting_y[i]][starting_x[i]] = ' ';

                gameData.players_local[i].slowed_down = 0;
                gameData.players_local[i].deaths++;
                gameData.players_local[i].coins_carried = 0;
            }
        }
    }

    for(int i = 0; i < PLAYERS; ++i) //update players' data, both in shared memory and local memory
    {
        if(gameData.players_shared[i])
        {
            sem_trywait(&gameData.players_shared[i]->access_data);
            int start_x = gameData.players_local[i].point.x - 2;
            int start_y = gameData.players_local[i].point.y - 2;

            for(int j = 0; j < P_SIGHT; ++j) // update maps
            {
                if(start_y + j < 0)
                    continue;
                if(start_y + j >= HEIGHT)
                    break;
                for(int k = 0; k < P_SIGHT; ++k)
                {
                    if(start_x + k < 0)
                        continue;
                    if(start_x + k >= WIDTH)
                        break;
                    gameData.players_shared[i]->player_board[start_y + j][start_x + k] = board[start_y + j][start_x + k];
                    if(gameData.players_local[i].campsite_discovered == false && board[start_y + j][start_x + k] == 'A')
                    {
                        gameData.players_local[i].campsite_discovered = true;
                        gameData.players_local[i].campsite_coords = gameData.campsite;
                    }

                    for(int l = 0; l < PLAYERS; ++l)
                    {
                        if(gameData.players_shared[l] && start_y + j == gameData.players_local[l].point.y && start_x + k == gameData.players_local[l].point.x)
                        {
                            char temp[2];
                            gameData.players_shared[i]->player_board[start_y + j][start_x + k] = itoa_base_10(gameData.players_local[l].ID, temp)[0];
                        }
                    }

                    for(int m = 0; m < BEASTS; ++m)
                    {
                        if(start_y + j == gameData.beasts[m].point.y && start_x + k == gameData.beasts[m].point.x)
                            gameData.players_shared[i]->player_board[start_y + j][start_x + k] = '*';
                    }
                }
            }
            gameData.players_shared[i]->campsite_discovered = gameData.players_local[i].campsite_discovered;
            if(gameData.players_local[i].campsite_discovered == true)
                gameData.players_shared[i]->campsite_coords = gameData.players_local[i].campsite_coords;

            gameData.players_shared[i]->ID = gameData.players_local[i].ID;
            gameData.players_shared[i]->PID = gameData.players_local[i].PID;
            gameData.players_shared[i]->server_PID = gameData.players_local[i].server_PID;
            gameData.players_shared[i]->type = gameData.players_local[i].type;
            gameData.players_shared[i]->spawn_coords = gameData.players_local[i].spawn_coords;
            gameData.players_shared[i]->point = gameData.players_local[i].point;
            gameData.players_shared[i]->round_counter = gameData.players_local[i].round_counter;
            gameData.players_shared[i]->direction = gameData.players_local[i].direction;
            gameData.players_shared[i]->slowed_down = gameData.players_local[i].slowed_down;
            gameData.players_shared[i]->deaths = gameData.players_local[i].deaths;
            gameData.players_shared[i]->coins_brought = gameData.players_local[i].coins_brought;
            gameData.players_shared[i]->coins_carried = gameData.players_local[i].coins_carried;
            sem_post(&gameData.players_shared[i]->access_data);
        }
    }
    pthread_mutex_unlock(&gameData.game_mutex);
}

//#pragma clang diagnostic pop
