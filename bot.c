#include "bot.h"
#include "config.h"

#include <stdio.h>
#include <time.h>

static SV
validate_user(SV sender)
{
        SV user = chop_by_delim(&sender, '!');
        SV user1 = chop_by_delim(&sender, '@');
        SV user2 = chop_by_delim(&sender, '.');

        if (!sv_eq(user1, user) || !sv_eq(user2, user) ||
            !sv_eq(sender, SV("tmi.twitch.tv")))
        {
               SV null = {0};                 
               return null;
        }

        return user;
}

int
handle_message(IRC *irc, SV sender, SV message)
{
        SV user = validate_user(sender);
        if (user.mem == NULL)
        {
                fprintf(stderr, "Error: unrecognised username: `%.*s`.\n",
                        sv_arg(sender));
                return 0;
        }

        if (sv_expect(&message, SV("`")) == 0)
        {
                SV command = chop_by_delim(&message, ' ');

                if (sv_eq(command, SV("ping")))
                {
                        printf("`%.*s` pinged the bot.\n", sv_arg(user));
                        irc_send_message(irc, SV("pong"));
                }

                else if (sv_eq(command, SV("date")))
                {
                        printf("`%.*s` requested the date.\n", sv_arg(user));
                        time_t unix_time = time(NULL);
                        const char *date = asctime(gmtime(&unix_time));
                        irc_send_message(irc, sv_from(date, strlen(date)));
                }

                else if (sv_eq(command, SV("weather")))
                {
                        printf("`%.*s` requested the weather.\n", sv_arg(user));
                        SV location = message;

                        if (location.count == 0)
                        {
                                //"set default date to london"
                                location = SV("london");
                        }

                        char cmd[500];
                        char buffer[500];

                        snprintf(cmd, sizeof(cmd),
                                 "curl -s 'wttr.in/%.*s?format=4'",
                                 sv_arg(location));

                        FILE *proc = popen(cmd, "r");
                        fgets(buffer, sizeof(buffer), proc);
                        pclose(proc);

                        irc_send_message(irc, sv_from(buffer, strlen(buffer)));
                }

                else if (sv_eq(command, SV("halt")))
                {
                        if (sv_eq(user, SV(NICK)))
                        {
                                printf("Shutting bot down...\n");
                                return -1;
                        }
                }
        }

        return 0;
}
