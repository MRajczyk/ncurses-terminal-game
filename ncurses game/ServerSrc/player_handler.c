#include "player_handler.h"

extern struct game_data gameData;
extern char board[HEIGHT][WIDTH];
extern int drops[HEIGHT][WIDTH];
extern sem_t sem_game_end;
extern int additional_treasure;
extern int beasts_alive;

int manage_player_shm(enum shm_action todo, int pid)
{
    char buff[15] = {"player_"};
    char temp[7] = {0};
    strcat(buff, itoa_base_10(pid, temp));

    if(todo == shm_delete)
        return shm_unlink(buff);
    else if(todo == shm_create)
        return shm_open(buff, O_RDWR | O_CREAT, 0666);
    else
        return -1;
}

void* player_conn(void* arg)
{
    while(1)
    {
        sem_wait(&gameData.players_info->waiting);
        if(gameData.finish == 1)
            return NULL;

        for(int i = 0; i < PLAYERS; ++i)
        {
            if(gameData.players_info->player[i].action == waiting)
            {
                pthread_mutex_lock(&gameData.game_mutex);

                int fd = manage_player_shm(shm_create, gameData.players_info->player[i].PID);
                if(fd == -1)
                {
                    kill(gameData.players_info->player[i].PID, SIGHUP);
                    gameData.players_info->player[i].action = empty;
                    puts("shm_create fail");
                    pthread_mutex_unlock(&gameData.game_mutex);
                    break;
                }
                if(ftruncate(fd, sizeof(struct player_data)) == -1)
                {
                    kill(gameData.players_info->player[i].PID, SIGHUP);
                    manage_player_shm(shm_delete, gameData.players_info->player[i].PID);
                    gameData.players_info->player[i].action = empty;
                    puts("ftruncate fail");
                    pthread_mutex_unlock(&gameData.game_mutex);
                    break;
                }
                gameData.players_shared[i] = mmap(NULL, sizeof(struct player_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if(*(int*)gameData.players_local == -1)
                {
                    kill(gameData.players_info->player[i].PID, SIGHUP);
                    manage_player_shm(shm_delete, gameData.players_info->player[i].PID);
                    gameData.players_info->player[i].action = empty;
                    puts("mmap fail");
                    pthread_mutex_unlock(&gameData.game_mutex);
                    break;
                }

                gameData.players_local[i].ID = i + 1;
                gameData.players_local[i].PID = gameData.players_info->player[i].PID;
                gameData.players_local[i].server_PID = gameData.PID;
                gameData.players_local[i].type = gameData.players_info->player[i].type;

                char buff[2];
                find_free_spot(itoa_base_10(i + 1, buff)[0], &gameData.players_local[i].point, 0);
                gameData.players_local[i].spawn_coords = gameData.players_local[i].point;
                gameData.players_local[i].round_counter = gameData.round_counter;
                gameData.players_local[i].direction = SIT;
                gameData.players_local[i].slowed_down = 0;
                gameData.players_local[i].deaths = 0;
                gameData.players_local[i].coins_brought = 0;
                gameData.players_local[i].coins_carried = 0;

                int start_x = gameData.players_local[i].point.x - 2;
                int start_y = gameData.players_local[i].point.y - 2;

                for(int j = 0; j < P_SIGHT; ++j)
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
                        gameData.players_local[i].player_board[start_y + j][start_x + k] = board[start_y + j][start_x + k];

                        for(int l = 0; l < PLAYERS; ++l)
                        {
                            if(gameData.players_shared[l] && start_y + j == gameData.players_local[l].point.y && start_x + k == gameData.players_local[l].point.x)
                            {
                                char temp[2];
                                gameData.players_local[i].player_board[start_y + j][start_x + k] = itoa_base_10(gameData.players_local[l].ID, temp)[0];
                            }
                        }

                        for(int m = 0; m < BEASTS; ++m)
                        {
                            if(start_y + j == gameData.beasts[m].point.y && start_x + k == gameData.beasts[m].point.x)
                                gameData.players_local[i].player_board[start_y + j][start_x + k] = '*';
                        }
                    }
                }

                sem_init(&gameData.players_shared[i]->player_moved, 1, 0);
                sem_init(&gameData.players_shared[i]->player_continue, 1, 1);
                sem_init(&gameData.players_shared[i]->access_data, 1, 1);

                gameData.players_info->player[i].action = playing;
                sem_post(&gameData.players_info->ask);
                sem_post(&gameData.players_info->leave);

                pthread_mutex_unlock(&gameData.game_mutex);
                break;
            }
        }
    }
}

void* player_disconn(void* arg)
{
    while(1)
    {
        sem_wait(&gameData.players_info->exit);
        if(gameData.finish == 1)
            return NULL;

        for(int i = 0; i < PLAYERS; ++i)
        {
            if(gameData.players_info->player[i].action == exitting)
            {
                pthread_mutex_lock(&gameData.game_mutex);
                //take care of players personalized shm
                munmap(gameData.players_shared[i], sizeof(struct player_data));
                manage_player_shm(shm_delete, gameData.players_info->player[i].PID);
                gameData.players_shared[i] = NULL;

                //clean remnants of a player in the the players_info structure
                sem_destroy(&gameData.players_shared[i]->player_moved);
                sem_destroy(&gameData.players_shared[i]->player_continue);
                sem_destroy(&gameData.players_shared[i]->access_data);
                gameData.players_info->player[i].action = empty;
                gameData.players_info->player[i].type = CPU;
                gameData.players_info->player[i].PID = -1;

                //give other players chance to ask
                sem_post(&gameData.players_info->leave);
                sem_post(&gameData.players_info->ask);

                pthread_mutex_unlock(&gameData.game_mutex);
            }
        }
    }
}
