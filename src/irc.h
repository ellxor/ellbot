#pragma once

#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "sv.h"

#define HOST "irc.chat.twitch.tv"
#define PORT "6697"
#define ADDR HOST":"PORT

typedef struct IRC IRC;

struct IRC
{
        int socket;
        SSL *ssl;
        SV channel;
};

int irc_connect(IRC *irc);
void irc_disconnect(IRC *irc);

int irc_send(IRC *irc, SV sv);
int irc_read(IRC *irc, char *buffer, int count);
void irc_send_message(IRC *irc, SV msg);
void irc_send_messages(IRC *irc, int count, ...);
void irc_join(IRC *irc, SV nick, SV pass);
