CC = gcc
CFLAGS = -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror
LDFLAGS = -lpthread -lrt

all: cliente impresora

cliente: cliente.c compartido.h
	$(CC) $(CFLAGS) -o cliente cliente.c $(LDFLAGS)

impresora: impresora.c compartido.h
	$(CC) $(CFLAGS) -o impresora impresora.c $(LDFLAGS)

clean:
	rm -f cliente impresora
	rm -f /dev/shm/spooler_impresion_buffer /dev/shm/sem.spooler_impresion_*

.PHONY: all clean
