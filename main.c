#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "bot.h"
#include "config.h"
#include "irc.h"
#include "sv.h"

static IRC irc = {0};
void sighandler(int);

int
main(int argc, char **argv)
{
        if (argc < 2)
        {
                puts("usage: ./ellbot <channel>");
                return 0;
        }

        if (irc_connect(&irc) == -1)
        {
                fprintf(stderr, "Fatal Error: shutting down...\n");
                exit(-1);
        };

        signal(SIGINT, sighandler);

        irc.channel = sv_from(argv[1], strlen(argv[1]));
        SV nickname = SV("NICK "NICK"\n");
        SV password = SV("PASS "PASS"\n");

        irc_send(&irc, password);
        irc_send(&irc, nickname);
        irc_send(&irc, SV("JOIN #"));
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
                        chop_right(&message, 2);      //"remove \r\n"

                        if (sv_expect(&username, SV(":"))       < 0 ||
                            sv_expect(&command,  SV("PRIVMSG")) < 0 ||
                            sv_expect(&message,  SV(":"))       < 0)
                        {
                                continue;
                        }

                        handle_message(&irc, username, message);
                }
        }
        while (1);

        return 0;
}

void
sighandler(int sn)
{
       printf("\n\nreceived sig(%d), freeing memory...\n", sn);
       irc_disconnect(&irc);
       exit(0);
}
