CFLAGS = -Wall -fPIC

build: tema1

tema1: tema1.o main.o hash.lib
	gcc $(CFLAGS) tema1.o main.o -o tema1 -L. -lhash

tema1.o: tema1.c
	gcc $(CFLAGS) -c tema1.c

main.o: main.c
	gcc $(CFLAGS) -c main.c
