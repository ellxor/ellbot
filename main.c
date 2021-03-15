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

        puts("Connected to Twitch!\n");

        SV channel  = SV("JOIN "CHANNEL"\n");
        SV nickname = SV("NICK "USER"\n");
        SV password = SV("PASS "PASS"\n");

        irc_send(&irc, password);
        irc_send(&irc, nickname);
        irc_send(&irc, channel);

        char buffer[4096] = {0};
        int count = 0;
        int size = 0;
        int exit = 0;

        do
        {
                size = irc_read(&irc, buffer, sizeof(buffer));
                if (size)
                {
                        SV message = sv_from(buffer, size);

                        if (expect(&message, SV("PING")) == 0)
                        {
                                printf("Server pinged!\n");
                                printf("Response = `PONG%.*s`\n", sv_arg(message));

                                irc_send(&irc, SV("PONG"));
                                irc_send(&irc, message);

                                continue;
                        }

                        SV username = chop_by_delim(&message, ' ');
                        SV command = chop_by_delim(&message, ' ');
                        chop_by_delim(&message, ' '); //"skip channel"

                        if (expect(&username, SV(":"))       < 0 ||
                            expect(&command,  SV("PRIVMSG")) < 0 ||
                            expect(&message,  SV(":"))       < 0)
                        {
                                continue;
                        }

                        //"remove trailing \r\n"
                        chop_right(&message, 2);

                        printf("'%.*s': '%.*s'\n\n", sv_arg(username), sv_arg(message));
                        count++;
                }
        }
        while (exit == 0);

        irc_disconnect(&irc);
        return 0;
}
