#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#include "bot.h"
#include "irc.h"
#include "sv.h"

static IRC irc = {0};
noreturn static void sighandler(int sig);

int
main(int argc, char **argv)
{
        if (argc < 2)
        {
                puts("usage: ./ellbot <channel>");
                return 0;
        }

        signal(SIGINT, sighandler);

        const char *nick = getenv("nick");
        const char *pass = getenv("pass");

        if (nick == NULL || pass == NULL)
        {
                fprintf(stderr, "missing environment variables\n");
                return -1;
        }

        if (irc_connect(&irc) == -1)
        {
                fprintf(stderr, "Fatal Error: shutting down...\n");
                exit(-1);
        }

        irc.channel = sv_from(argv[1], strlen(argv[1]));
        irc_join(&irc, sv_from(nick, strlen(nick)),
                       sv_from(pass, strlen(pass)));

        puts("Connected to Twitch!\n");

        char buffer[4096] = {0};
        int size = 0;

        do
        {
                size = irc_read(&irc, buffer, sizeof(buffer));
                if (size)
                {
                        SV message = sv_from(buffer, size);

                        if (sv_expect(&message, SV("PING")))
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

                        if (sv_expect(&username, SV(":"))       &&
                            sv_expect(&command,  SV("PRIVMSG")) &&
                            sv_expect(&message,  SV(":")))
                        {
                                handle_message(&irc, username, message);
                        }
                }
        }
        while (1);
}

static void
sighandler(int sig)
{
       printf("\n\nReceived sig(%d), freeing memory...\n", sig);
       irc_disconnect(&irc);
       puts("Exiting successfully");
       exit(0);
}
