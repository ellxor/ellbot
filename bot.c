#include "bot.h"
#include "config.h"

#include <assert.h>
#include <curl/curl.h>
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
static int _rnd(IRC *irc, SV sender, SV arg);
static int kill(IRC *irc, SV sender, SV arg);

static struct command
COMMANDS[64] =
{
        [0x0C] = {.name = SV("cmds"), .action = cmds},
        [0x11] = {.name = SV("kill"), .action = kill},
        [0x23] = {.name = SV("date"), .action = date},
        [0x2A] = {.name = SV("rand"), .action = _rnd},
        [0x33] = {.name = SV("ping"), .action = ping},
        [0x36] = {.name = SV("wttr"), .action = wttr},
};

static SV
validate_user(SV sender)
{
        SV user =  chop_by_delim(&sender, '!');
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
sv_cmp(const void *a, const void *b)
{
        SV sa = *(SV *)a;
        SV sb = *(SV *)b;

        int len = (sa.count < sb.count)
                ? sa.count
                : sb.count;

        return strncmp(sa.mem, sb.mem, len);
}

static int
cmds(IRC *irc, SV sender, SV arg)
{
        SV buff[64];
        int len = 0;

        for (int i = 0; i < 64; i++)
        {
                SV name = COMMANDS[i].name;

                if (name.mem != NULL)
                {
                        buff[len++] = name;
                }
        }

        qsort(buff, len, sizeof(SV), sv_cmp);

        char buffer[500];
        char *writer = buffer;

        for (int i = 0; i < len; i++)
        {
                writer += snprintf(writer,
                                   500 - (writer - buffer),
                                   "%.*s, ", sv_arg(buff[i])
                                  );
        }

        irc_send_message(irc,
                sv_from(buffer, writer - buffer - 2));
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

static size_t
curl_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
        assert(size == 1 && "size must be 1");

        SV *sv = (SV *)data;
        sv->mem = ptr;
        sv->count = nmemb;

        return nmemb;
}


static int
wttr(IRC *irc, SV sender, SV arg)
{
        if (arg.count == 0)
        {
                irc_send_message(irc,
                        SV("error: `wttr` expects 1 arg\n"));
                return 0;
        }

        CURL *curl = curl_easy_init();
        if (curl != NULL)
        {
                char url[100] = {0};
                snprintf(url, sizeof(url),
                         "https://wttr.in/%.*s?format=4", sv_arg(arg));

                SV data = {0};

                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);
                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                irc_send_message(irc, data);
        }

        return 0;
}

static int
_rnd(IRC *irc, SV sender, SV args)
{
        SV arg = chop_by_delim(&args, ' ');
        uint32_t min = 0, max = 0, count = 0;

        while (arg.count > 0 && count < 2)
        {
                if ((min = max, max = sv_parse_uint(arg)) == -1)
                {
                        irc_send_messages(irc, 3,
                                          SV("error: invalid int `"),
                                          arg,
                                          SV("`"));
                        return 0;
                }

                arg = chop_by_delim(&args, ' ');
                count++;
        }

        if (count == 0)
        {
                irc_send_message(irc,
                        SV("error: `rand` expects 1-2 args\n"));
                return 0;
        }

        srand(time(NULL));
        double r = (double)rand() / RAND_MAX;
        uint32_t s = min + r * (0.99 + max - min);

        char buffer[20];
        int len = sprintf(buffer, "%d\n", s);

        irc_send_message(irc, sv_from(buffer, len));
        return 0;
}

static int
kill(IRC *irc, SV sender, SV arg)
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
