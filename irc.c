#include "irc.h"
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

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

int 
irc_connect(int *socket_ptr)
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

        fprintf(stderr, "Failed to connect to '"HOST":"PORT"': %m\n");
        close(socket);
        return -1;
}

void
irc_disconnect(int *socket_ptr)
{
        close(*socket_ptr);
}
