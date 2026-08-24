# Estructura general de un Makefile:
# objetivo: dependencia1 dependencia2 ...
# <TAB> comando1
# <TAB> comando2

CC=gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror

.PHONY: all clean

all: programa

programa: main.o cadenas.o
	$(CC) -o programa main.o cadenas.o 

main.o: main.c cadenas.h
	$(CC) $(CFLAGS) -c main.c 

cadenas.o: cadenas.c cadenas.h
	$(CC) $(CFLAGS) -c cadenas.c 

clean:
	rm -f *.o programa