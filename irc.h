#pragma once
#define HOST "irc.chat.twitch.tv"
#define PORT "6697"

int irc_connect(int *socket_ptr);
void irc_disconnect(int *socket_ptr);
