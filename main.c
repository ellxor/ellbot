#include <stdio.h>
#include "config.h"
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

        #define channel "JOIN "CHANNEL"\n"
        #define nickname "NICK "USER"\n"
        #define password "PASS "PASS"\n"

        SSL_write(irc.ssl, password, sizeof(password) - 1);
        SSL_write(irc.ssl, nickname, sizeof(nickname) - 1);
        SSL_write(irc.ssl, channel, sizeof(channel) - 1);

        char buffer[4096] = {0};
        int count = 0;
        int size = 0;

        do
        {
                size = SSL_read(irc.ssl, buffer, sizeof(buffer));
                if (size)
                {
                        printf("\n\nRead %d bytes:\n", size);
                        printf("%.*s", size, buffer);
                        count++;
                }
        }
        while (count < 6);

        irc_disconnect(&irc);
        return 0;
}
