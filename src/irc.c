#include "irc.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#ifndef IPPROTO_TCP
#include <linux/in.h>
#endif

/// TCP/IP ///

static int
open_socket(struct addrinfo *info)
{
        int sockfd = socket(info->ai_family,
                            info->ai_socktype,
                            info->ai_protocol
                           );

        if (sockfd == -1)
        {
                fprintf(stderr, "Failed to open socket: %m\n");
                exit(-1);
        }

        return sockfd;
}

static int 
connect_socket(int *socket_ptr)
{
        struct addrinfo info = {
                .ai_family = AF_INET,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = IPPROTO_TCP,
        };

        int socket = open_socket(&info);
        struct addrinfo *addr = NULL;

        if (getaddrinfo(HOST, PORT, &info, &addr) < 0)
        {
                fprintf(stderr, "Failed to get address of '"HOST"': %m\n");
                close(socket);
                return -1;
        }

        while (addr != NULL)
        {
                if (connect(socket, addr->ai_addr, addr->ai_addrlen) == 0)
                {
                        *socket_ptr = socket;
                        return 0;
                }

                addr = addr->ai_next;
        }

        fprintf(stderr, "Failed to connect to '"ADDR"': %m\n");
        close(socket);
        return -1;
}

/// OPENSSL ///

static IRC
irc_from_socket(int *socket)
{
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        SSL_CTX *context = SSL_CTX_new(TLS_client_method());

        if (context == NULL)
        {
                fprintf(stderr, "SSL initialisation failed\n");
                ERR_print_errors_fp(stderr);
                close(*socket);
                exit(-1);
        }

        IRC irc = {
                .socket = *socket,
                .ssl = SSL_new(context),
        };

        SSL_set_fd(irc.ssl, irc.socket);
        if (SSL_connect(irc.ssl) < 0)
        {
                fprintf(stderr, "Could not connect to '"ADDR"' via SSL\n");
                ERR_print_errors_fp(stderr);
                irc_disconnect(&irc);
                exit(-1);
        }

        return irc;
}

/// IRC WRAPPERS ///

int
irc_connect(IRC *irc)
{
        int socket = 0;

        if (connect_socket(&socket) == -1)
        {
                fprintf(stderr, "Fatal Error: could not find '"ADDR"'\n");
                return -1;
        }

        *irc = irc_from_socket(&socket);
        return 0;
}

void
irc_disconnect(IRC *irc)
{
        if (irc->ssl != NULL)
        {
                SSL_set_shutdown(irc->ssl, SSL_SENT_SHUTDOWN
                                         | SSL_RECEIVED_SHUTDOWN);
                SSL_shutdown(irc->ssl);
                SSL_free(irc->ssl);
        }

        close(irc->socket);
}

int
irc_send(IRC *irc, SV sv)
{
        return SSL_write(irc->ssl, sv.mem, sv.count);
}

int
irc_read(IRC *irc, char *buffer, int count)
{
        return SSL_read(irc->ssl, buffer, count);
}

void
irc_send_message(IRC *irc, SV msg)
{
        irc_send_messages(irc, 1, msg);
}

void
irc_send_messages(IRC *irc, int count, ...)
{
        irc_send(irc, SV("PRIVMSG #"));
        irc_send(irc, irc->channel);
        irc_send(irc, SV(" :"));

        va_list args;
        va_start(args, count);

        for (int i = 0; i < count; i++)
        {
                irc_send(irc, va_arg(args, SV));
        }

        va_end(args);
        irc_send(irc, SV("\r\n"));
}

void 
irc_join(IRC *irc, SV nick, SV pass)
{
        irc_send(irc, SV("PASS "));
        irc_send(irc, pass);
        irc_send(irc, SV("\n"));

        irc_send(irc, SV("NICK "));
        irc_send(irc, nick);
        irc_send(irc, SV("\n"));

        irc_send(irc, SV("JOIN #"));
        irc_send(irc, irc->channel);
        irc_send(irc, SV("\n"));
}
