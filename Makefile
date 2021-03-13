CC = clang

CFLAGS = -O3 -s
DEBUG = -O2 -fsanitize=undefined -Wall -Wextra

OBJS = -lssl -lcrypto

default:; $(CC) *.c -o main $(CFLAGS) $(OBJS)
debug:; $(CC) *.c -o main $(DEBUG) $(OBJS)
