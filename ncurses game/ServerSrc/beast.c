#include "beast.h"

extern struct game_data gameData;
extern char board[HEIGHT][WIDTH];
extern int drops[HEIGHT][WIDTH];
extern sem_t sem_game_end;
extern int additional_treasure;
extern int beasts_alive;

void beast_random_move(int i)
{
    int mov = rand() % 5;
    switch(mov)
    {
        case 0:
            gameData.beasts[i].direction = UP;
            break;
        case 1:
            gameData.beasts[i].direction = DOWN;
            break;
        case 2:
            gameData.beasts[i].direction = RIGHT;
            break;
        case 3:
            gameData.beasts[i].direction = LEFT;
            break;
        case 4:
            gameData.beasts[i].direction = SIT;
            break;
    }
}

_Noreturn void* beast_mov(void* arg)
{
    int i = *(int*)arg;
    while(1)
    {
        sem_wait(&gameData.beasts[i].beast_move);
        if(gameData.beasts[i].point.x > 0)
        {
            int closest_player = 4;
            double dist = sqrt(HEIGHT * HEIGHT + WIDTH * WIDTH);

            for(int j = 0; j < PLAYERS; ++j)
            {
                if(gameData.players_shared[j])
                {
                    int temp = sqrt(pow(gameData.players_local[j].point.y - gameData.beasts[i].point.y, 2) + pow(gameData.players_local[j].point.x - gameData.beasts[i].point.x, 2));
                    if(temp < dist)
                    {
                        closest_player = j;
                        dist = temp;
                    }
                }
            }
            if(dist >= 3)
                closest_player = 4;

            if(closest_player < 4)
            {
                int diff_y = gameData.players_local[closest_player].point.y - gameData.beasts[i].point.y;
                int diff_x = gameData.players_local[closest_player].point.x - gameData.beasts[i].point.x;

                if(diff_y == 0 && diff_x == 0)
                    gameData.beasts[i].direction = SIT;
                if(diff_y < 0)
                {
                    if(board[gameData.beasts[i].point.y - 1][gameData.beasts[i].point.x] != '|')
                        gameData.beasts[i].direction = UP;
                    else if(diff_x > 0)
                    {
                        if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x + 1] != '|')
                            gameData.beasts[i].direction = RIGHT;
                        else
                            beast_random_move(i);
                    }
                    else if(diff_x < 0)
                    {
                        if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x - 1] != '|')
                            gameData.beasts[i].direction = LEFT;
                        else
                            beast_random_move(i);
                    }
                    else
                        beast_random_move(i);
                }
                else if(diff_y > 0)
                {
                    if(board[gameData.beasts[i].point.y + 1][gameData.beasts[i].point.x] != '|')
                        gameData.beasts[i].direction = DOWN;
                    else if(diff_x > 0)
                    {
                        if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x + 1] != '|')
                            gameData.beasts[i].direction = RIGHT;
                        else
                            beast_random_move(i);
                    }
                    else if(diff_x < 0)
                    {
                        if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x - 1] != '|')
                            gameData.beasts[i].direction = LEFT;
                        else
                            beast_random_move(i);
                    }
                    else
                        beast_random_move(i);
                }
                else if(diff_x > 0)
                {
                    if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x + 1] != '|')
                        gameData.beasts[i].direction = RIGHT;
                    else
                        beast_random_move(i);
                }
                else if(diff_x < 0)
                {
                    if(board[gameData.beasts[i].point.y][gameData.beasts[i].point.x - 1] != '|')
                        gameData.beasts[i].direction = LEFT;
                    else
                        beast_random_move(i);
                }
                else
                    beast_random_move(i);
            }
            else
                beast_random_move(i);
        }
        sem_post(&gameData.beasts[i].beast_moved);
    }
}
