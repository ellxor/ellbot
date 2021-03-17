#include "bot.h"
#include "config.h"

#include <stdio.h>
#include <time.h>

struct command
{
        SV name;
        int (*action)(IRC *irc, SV sender, SV arg);
};

static int cmds(IRC *irc, SV sender, SV arg);
static int ping(IRC *irc, SV sender, SV arg);
static int date(IRC *irc, SV sender, SV arg);
static int wttr(IRC *irc, SV sender, SV arg);
static int halt(IRC *irc, SV sender, SV arg);

static struct command COMMANDS[64] =
{
        [0x0C] = {.name = SV("cmds"), .action = cmds},
        [0x23] = {.name = SV("date"), .action = date},
        [0x2E] = {.name = SV("halt"), .action = halt},
        [0x33] = {.name = SV("ping"), .action = ping},
        [0x36] = {.name = SV("wttr"), .action = wttr},
};

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

static int
cmds(IRC *irc, SV sender, SV arg)
{
        #define prefix "commands: "
        char buffer[256] = prefix;
        char *writer = buffer + sizeof(prefix) - 1;

        for (int i = 0; i < 64; i++)
        {
                SV name = COMMANDS[i].name;
                if (name.mem != NULL)
                {
                        writer += sprintf(writer, "`%.*s` ",
                                          sv_arg(name));

                        if (writer - buffer > 200)
                        {
                                fprintf(stderr, "Error: buffer limit exceeded "
                                                "when displaying commands\n");
                        };
                }
        }

        *writer = 0;

        SV msg = sv_from(buffer, writer - buffer);
        irc_send_message(irc, msg);

        return 0;
}

static int
ping(IRC *irc, SV sender, SV arg)
{
        irc_send_message(irc, SV("pong"));
        return 0;
}

static int
date(IRC *irc, SV sender, SV arg)
{
        time_t unix_time = time(NULL);
        const char *date = asctime(gmtime(&unix_time));
        irc_send_message(irc, sv_from(date, strlen(date)));
        return 0;
}

static int
wttr(IRC *irc, SV sender, SV arg)
{
        if (arg.count == 0)
        {
                //"set default loc to london"
                arg = SV("london");
        }

        char cmd[500];
        char buffer[500];

        // TODO: "sanitize input: urgent!"
        snprintf(cmd, sizeof(cmd),
                 "curl -s 'wttr.in/%.*s?format=4'",
                 sv_arg(arg));

        FILE *proc = popen(cmd, "r");
        fgets(buffer, sizeof(buffer), proc);
        pclose(proc);

        irc_send_message(irc, sv_from(buffer, strlen(buffer)));
        return 0;
}

static int
halt(IRC *irc, SV sender, SV arg)
{
        if (sv_eq(sender, SV(NICK)))
        {
                printf("Halting bot at the request of `%.*s`\n",
                       sv_arg(sender));
                return -1;
        }

        return 0;
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

                uint32_t hash = 0x3F & sv_hash(command);
                SV check = COMMANDS[hash].name;

                if (check.mem != NULL && sv_eq(command, check))
                {
                        int code = COMMANDS[hash].action(
                                        irc, user, message
                                   );

                        if (code) return code;
                }
        }

        return 0;
}
