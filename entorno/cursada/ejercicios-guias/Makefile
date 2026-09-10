CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lpthread -lrt
else
    LDFLAGS = -lpthread
endif

.PHONY: all clean

all: recepcion cocina

recepcion: recepcion.o pedidos.o
	$(CC) $(CFLAGS) -o recepcion recepcion.o pedidos.o $(LDFLAGS)

cocina: cocina.o pedidos.o
	$(CC) $(CFLAGS) -o cocina cocina.o pedidos.o $(LDFLAGS)

recepcion.o: recepcion.c pedidos.h
	$(CC) $(CFLAGS) -c recepcion.c

cocina.o: cocina.c pedidos.h
	$(CC) $(CFLAGS) -c cocina.c

pedidos.o: pedidos.c pedidos.h
	$(CC) $(CFLAGS) -c pedidos.c

clean:
	rm -f *.o recepcion cocina pedidos.txt
	rm -f /dev/shm/sem.sem_pedidos_mutex /dev/shm/sem.sem_pedidos_items 2>/dev/null || true


