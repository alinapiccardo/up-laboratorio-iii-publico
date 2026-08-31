CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror

.PHONY: all clean

all: recepcion cocina

recepcion: recepcion.o pedidos.o
	$(CC) $(CFLAGS) -o recepcion recepcion.o pedidos.o

cocina: cocina.o pedidos.o
	$(CC) $(CFLAGS) -o cocina cocina.o pedidos.o

recepcion.o: recepcion.c pedidos.h
	$(CC) $(CFLAGS) -c recepcion.c

cocina.o: cocina.c pedidos.h
	$(CC) $(CFLAGS) -c cocina.c

pedidos.o: pedidos.c pedidos.h
	$(CC) $(CFLAGS) -c pedidos.c

clean:
	rm -f *.o recepcion cocina pedidos.txt
