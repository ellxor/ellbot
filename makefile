SRC = src/*.c
LIB = -lssl -lcrypto -lcurl -lm
FLAGS = -O3 -s -Wall -Wextra

ellbot:
	gcc -o ellbot $(SRC) $(LIB) $(FLAGS)
