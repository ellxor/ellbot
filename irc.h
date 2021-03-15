#pragma once

#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "stringview.h"

#define HOST "irc.chat.twitch.tv"
#define PORT "6697"
#define ADDR HOST":"PORT

typedef struct
{
        int socket;
        SSL *ssl;
}
IRC;

int irc_connect(IRC *irc);
void irc_disconnect(IRC *irc);

int irc_send(IRC *irc, SV sv);
int irc_read(IRC *irc, char *buffer, int count);
