#include <stdio.h>
#include "irc.h"

int
main(int argc, char **argv)
{
        int socket;

        irc_connect(&socket);
        puts("Connection successful!");
        irc_disconnect(&socket);

        return 0;
}
