#include <stdio.h>
#include "irc.h"

int
main(int argc, char **argv)
{
        IRC irc = {0};

        if (irc_connect(&irc) == -1)
        {
                fprintf(stderr, "Fatal Error: shutting down...\n");
                exit(-1);
        };

        puts("SSL Connection successful!");
        printf("%d | %p\n", irc.socket, irc.ssl);

        irc_disconnect(&irc);
        return 0;
}
