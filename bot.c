#include "bot.h"
#include "calc.h"

#include <assert.h>
#include <curl/curl.h>
#include <stdio.h>
#include <time.h>

struct command
{
        SV name;
        void (*action)(IRC *irc, SV sender, SV arg);
};

static void cmds(), date(), rnd(), src(), ping(),
            wttr(), calc(), dots();

// must be power of 2
#define COMMAND_COUNT 64

static struct command
COMMANDS[COMMAND_COUNT] =
{
        [0x0C] = {.name = SV("cmds"), .action = cmds},
        [0x23] = {.name = SV("date"), .action = date},
        [0x29] = {.name = SV("rnd"),  .action = rnd },
        [0x2D] = {.name = SV("src"),  .action = src },
        [0x33] = {.name = SV("ping"), .action = ping},
        [0x36] = {.name = SV("wttr"), .action = wttr},
        [0x38] = {.name = SV("calc"), .action = calc},
        [0x3F] = {.name = SV("dots"), .action = dots},
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

static void
cmds(IRC *irc, SV sender, SV arg)
{
        (void)sender;
        (void)arg;

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
}

static void
ping(IRC *irc, SV sender, SV arg)
{
        (void)sender;
        (void)arg;

        irc_send_message(irc, SV("pong"));
}

static void
date(IRC *irc, SV sender, SV arg)
{
        (void)sender;
        (void)arg;

        time_t unix_time = time(NULL);
        const char *date = asctime(gmtime(&unix_time));
        irc_send_message(irc, sv_from(date, strlen(date)));
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


static void
wttr(IRC *irc, SV sender, SV arg)
{
        (void)sender;

        if (arg.count == 0)
        {
                irc_send_message(irc,
                        SV("error: `wttr` expects 1 arg\n"));
                return;
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
}

static void
rnd(IRC *irc, SV sender, SV args)
{
        (void)sender;

        SV arg = chop_by_delim(&args, ' ');
        uint32_t min = 0, max = 0, count = 0;

        while (arg.count > 0 && count < 2)
        {
                if ((min = max, max = sv_parse_uint(arg)) == (uint32_t)-1)
                {
                        irc_send_messages(irc, 3,
                                          SV("error: invalid int `"),
                                          arg,
                                          SV("`"));
                        return;
                }

                arg = chop_by_delim(&args, ' ');
                count++;
        }

        if (count == 0)
        {
                irc_send_message(irc,
                        SV("error: `rand` expects 1-2 args\n"));
                return;
        }

        srand(time(NULL));
        double r = (double)rand() / RAND_MAX;
        uint32_t s = min + r * (0.99 + max - min);

        char buffer[20];
        int len = sprintf(buffer, "%d\n", s);

        irc_send_message(irc, sv_from(buffer, len));
        return;
}

static void
src(IRC *irc, SV sender, SV arg)
{
        (void)sender;
        (void)arg;

        SV msg = SV("source code: https://github.com/ellxor/ellbot");
        irc_send_message(irc, msg);
}

static void
calc(IRC *irc, SV sender, SV arg)
{
        (void)sender;
        (void)arg;

        SV err = {0};
        SV res = eval(arg, &err);

        if (err.mem == NULL)
        {
                irc_send_message(irc, res);
        }

        else
        {
                irc_send_messages(irc, 2, res, err);
        }
}

static void
dots(IRC *irc, SV sender, SV arg)
{
        (void)sender;
        (void)arg;

        SV msg = SV("dot files: https://github.com/ellxor/dots");
        irc_send_message(irc, msg);
}

void
handle_message(IRC *irc, SV sender, SV message)
{
        SV user = validate_user(sender);
        if (user.mem == NULL)
        {
                fprintf(stderr, "Error: unrecognised username: `%.*s`.\n",
                        sv_arg(sender));
                return;
        }

        if (sv_expect(&message, SV("`")) == 0)
        {
                SV command = chop_by_delim(&message, ' ');

                uint32_t hash = sv_hash(command) & (COMMAND_COUNT - 1);
                SV check = COMMANDS[hash].name;

                if (check.mem != NULL && sv_eq(command, check))
                {
                        COMMANDS[hash].action(
                                irc, user, message
                        );
                }
        }
}
