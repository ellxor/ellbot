#include <stdio.h>
#include "irc.h"

int
main(int argc, char **argv)
{
        IRC irc = {0};

        irc_connect(&irc);

        puts("SSL Connection successful!");
        printf("%d | %p\n", irc.socket, irc.ssl);

        irc_disconnect(&irc);

        return 0;
}
