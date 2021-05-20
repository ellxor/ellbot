CC = clang

SRC = src/*.c
LIB = -lssl -lcrypto -lcurl -lm
FLAGS = -O3 -s -Wall -Wextra -Wno-multichar

ellbot: src/*.c src/*.h
	$(CC) -o $@ $(SRC) $(LIB) $(FLAGS)
