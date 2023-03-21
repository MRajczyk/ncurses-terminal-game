#include "connection_man.h"

extern struct player_data* ReceivedData;
extern struct conn_players* ConnectedPlayers;

extern pthread_t board_handler, kb_handler;

int join_server()
{
    mvprintw(HEIGHT + 2, 1, "Establishing connection with server...");
    refresh();
    sem_wait(&ConnectedPlayers->ask);

    int pid = getpid();
    for(int i = 0; i < PLAYERS + 1; ++i)
    {
        if(i == PLAYERS)
        {
            attron(CO_DEFAULT);
            sem_post(&ConnectedPlayers->ask);
            wclear(stdscr);
            mvprintw(HEIGHT + 2, 1, "Server is full.");
            refresh();
            return 0;
        }
        if(ConnectedPlayers->player[i].action == empty)
        {
            ConnectedPlayers->player[i].action = waiting;
            ConnectedPlayers->player[i].PID = pid;
            ConnectedPlayers->player[i].type = HUMAN;
            sem_post(&ConnectedPlayers->waiting);
            break;
        }
    }
    int retries = 0;
    while(1)
    {
        wclear(stdscr);
        attron(CO_DEFAULT);
        mvprintw(HEIGHT + 2, 1, "Joining...");
        refresh();
        usleep(500 * 1000);
        if(sem_trywait(&ConnectedPlayers->leave) == 0)
            break;
        if(retries++ >= 30)
        {
            wclear(stdscr);
            attron(CO_DEFAULT);
            mvprintw(HEIGHT + 2, 1, "Could not join the server.");
            refresh();

            munmap(ConnectedPlayers, sizeof(struct conn_players));
            return 0;
        }
    }
    char buff[15] = {"player_"};
    char temp[7] = {0};
    strcat(buff, itoa_base_10(pid, temp));

    int fd = shm_open(buff, O_RDWR, 0666);
    if(fd == -1)
    {
        puts("shm_open(player)");
        return 0;
    }
    if(ftruncate(fd, sizeof(struct player_data)) == -1)
    {
        puts("ftruncate(player)");
        return 0;
    }
    ReceivedData = mmap(NULL, sizeof(struct player_data), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if(*(int*)ReceivedData == -1)
    {
        puts("mmap(player)");
        return 0;
    }

    attron(CO_DEFAULT);
    mvprintw(HEIGHT + 2, 1, "           ");
    refresh();

    return 1;
}

int exit_server()
{
    pthread_cancel(board_handler);
    if(kill(ReceivedData->server_PID, 0) != 0)
    {
        wclear(stdscr);
        mvprintw(WIDTH / 2, HEIGHT / 2, "CONNECTION WITH SERVER DOWN. LEAVING...");
        refresh();
        usleep(2000 * 1000);
        munmap(ConnectedPlayers, sizeof(struct conn_players));
        system("reset");
        pthread_cancel(board_handler);
        return 1;
    }
    sem_wait(&ConnectedPlayers->ask);
    wclear(stdscr);

    for(int i = 0; i < PLAYERS + 1; ++i)
    {
        if(i == PLAYERS)
        {
            sem_post(&ConnectedPlayers->ask);
            wclear(stdscr);
            mvprintw(HEIGHT / 2, WIDTH / 2, "how..?!");  //if this ever happens, dear god....
            refresh();
            munmap(ConnectedPlayers, sizeof(struct conn_players));
            return -1;
        }
        if(ConnectedPlayers->player[i].PID == getpid())
        {
            ConnectedPlayers->player[i].action = exitting;
            sem_post(&ConnectedPlayers->exit);
            break;
        }
    }

    int waiting = 0;
    while(1)
    {
        wclear(stdscr);
        attron(CO_DEFAULT);
        mvprintw(HEIGHT + 2, 1, "Exitting...");
        refresh();
        usleep(500 * 1000);
        if(sem_trywait(&ConnectedPlayers->leave) == 0)
            break;
        if(++waiting > 20)
        {
            wclear(stdscr);
            attron(CO_DEFAULT);
            mvprintw(HEIGHT + 2, 1, "Could not exit the server.");
            refresh();

            munmap(ConnectedPlayers, sizeof(struct conn_players));
            return -1;
        }
    }

    munmap(ConnectedPlayers, sizeof(struct conn_players));
    system("reset");
    return 1;
}
