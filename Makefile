CC = clang

CFLAGS = -O3 -s
DEBUG = -O2 -fsanitize=undefined

default:; $(CC) *.c -o main $(CFLAGS)
debug:; $(CC) *.c -o main $(DEBUG)
