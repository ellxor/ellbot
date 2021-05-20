CC = gcc

SRC = src/*.c
LIB = -lssl -lcrypto -lcurl -lm
FLAGS = -O3 -s -Wall -Wextra -Wno-multichar

ellbot:
	$(CC) -o $@ $(SRC) $(LIB) $(FLAGS)
