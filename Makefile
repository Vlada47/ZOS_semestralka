CC = gcc
CFLAGS = -I ./headers/ -lm -lpthread -Wall -O3
BIN = FAT_Defragmentator
OBJ = source/main.o source/global_functions.o source/worker_functions.o

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)
	rm -f $(OBJ)