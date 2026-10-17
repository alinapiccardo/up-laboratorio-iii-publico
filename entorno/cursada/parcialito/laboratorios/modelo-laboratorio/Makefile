CC = gcc
CFLAGS = -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror
LDFLAGS = -lpthread -lrt

all: recepcion analista

recepcion: recepcion.c compartido.h
	$(CC) $(CFLAGS) -o recepcion recepcion.c $(LDFLAGS)

analista: analista.c compartido.h
	$(CC) $(CFLAGS) -o analista analista.c $(LDFLAGS)

clean:
	rm -f recepcion analista
	rm -f /dev/shm/laboratorio_muestras_buffer /dev/shm/sem.laboratorio_muestras_*

.PHONY: all clean
