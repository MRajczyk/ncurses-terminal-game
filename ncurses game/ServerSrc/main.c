#include "server.h"

int main()
{
    if(set_game() == -1)
    {
        puts("Failed to start the server...");
        return 1;
    }
    game();

    return 0;
}
