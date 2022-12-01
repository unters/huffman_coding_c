CC = gcc
CFLAGS = -c

all: huffman_encoding

debug: main.o huffman.o
	$(CC) main.o huffman.o -g -o main

huffman_encoding: main.o huffman.o
	$(CC) main.o huffman.o -o main

main.o: main.c
	$(CC) $(CFLAGS) main.c

huffman.o: huffman.c
	$(CC) $(CFLAGS) huffman.c

clean:
	rm -rf *.o
