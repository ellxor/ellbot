#include <stdio.h>

#include "bot.h"
#include "config.h"
#include "irc.h"
#include "sv.h"

int
main(int argc, char **argv)
{
        if (argc < 2)
        {
                puts("usage: ./ellbot <channel>");
                return 0;
        }

        IRC irc = {0};

        if (irc_connect(&irc) == -1)
        {
                fprintf(stderr, "Fatal Error: shutting down...\n");
                exit(-1);
        };

        char chan_buffer[100] = "#";
        int len = 1;

        len += snprintf(chan_buffer + 1,
                        sizeof(chan_buffer) - 1,
                        "%s", argv[1]);

        irc.channel = sv_from(chan_buffer, len);
        printf("Channel = `%.*s`\n", sv_arg(irc.channel));

        SV nickname = SV("NICK "NICK"\n");
        SV password = SV("PASS "PASS"\n");

        irc_send(&irc, password);
        irc_send(&irc, nickname);
        irc_send(&irc, SV("JOIN "));
        irc_send(&irc, irc.channel);
        irc_send(&irc, SV("\n"));

        puts("Connected to Twitch!\n");

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
