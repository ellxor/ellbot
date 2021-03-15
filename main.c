#include <stdio.h>

#include "config.h"
#include "irc.h"
#include "stringview.h"

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

        SV channel  = SV("JOIN "CHANNEL"\n");
        SV nickname = SV("NICK "USER"\n");
        SV password = SV("PASS "PASS"\n");

        irc_send(&irc, password);
        irc_send(&irc, nickname);
        irc_send(&irc, channel);

        char buffer[4096] = {0};
        int count = 0;
        int size = 0;

        do
        {
                size = irc_read(&irc, buffer, sizeof(buffer));
                if (size)
                {
                        SV data = sv_from(buffer, size);
                        printf("\n\nRead %d bytes:\n", size);
                        printf("%.*s", data.count, data.mem);
                        count++;
                }

                // EXAMPLE MESSAGE:
                //"Read 88 bytes:"
                //":syphoxy!syphoxy@syphoxy.tmi.twitch.tv PRIVMSG #tsoding :inb4 \"why not copy and paste\""
        }
        while (count < 10);

        irc_disconnect(&irc);
        return 0;
}
