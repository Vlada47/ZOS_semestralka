CC = gcc
CFLAGS = -I ./headers/ -lm -lpthread -Wall -O3
BIN = FAT_Worker
OBJ = source/main.o source/worker.o

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)
	rm -f $(OBJ)