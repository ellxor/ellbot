#include <stdio.h>

#include "bot.h"
#include "config.h"
#include "irc.h"
#include "sv.h"

int
main(int argc, char **argv)
{
        IRC irc = {0};

        if (irc_connect(&irc) == -1)
        {
                fprintf(stderr, "Fatal Error: shutting down...\n");
                exit(-1);
        };

        irc.channel = SV(CHANNEL);

        puts("Connected to Twitch!\n");

        SV     join = SV("JOIN "CHANNEL"\n");
        SV nickname = SV("NICK "USER"\n");
        SV password = SV("PASS "PASS"\n");

        irc_send(&irc, password);
        irc_send(&irc, nickname);
        irc_send(&irc, join);

        char buffer[4096] = {0};
        int size = 0;

        do
        {
                size = irc_read(&irc, buffer, sizeof(buffer));
                if (size)
                {
                        SV message = sv_from(buffer, size);

                        if (sv_expect(&message, SV("PING")) == 0)
                        {
                                puts("Server pinged and bot ponged!");
                                irc_send(&irc, SV("PONG"));
                                irc_send(&irc, message);
                                continue;
                        }

                        SV username = chop_by_delim(&message, ' ');
                        SV command = chop_by_delim(&message, ' ');
                        chop_by_delim(&message, ' '); //"skip channel"
                        chop_right(&message, 2);

                        if (sv_expect(&username, SV(":"))       < 0 ||
                            sv_expect(&command,  SV("PRIVMSG")) < 0 ||
                            sv_expect(&message,  SV(":"))       < 0)
                        {
                                continue;
                        }

                        int code = handle_message(&irc, username, message);
                        if (code == -1) break;
                }
        }
        while (1);

        irc_disconnect(&irc);
        return 0;
}
