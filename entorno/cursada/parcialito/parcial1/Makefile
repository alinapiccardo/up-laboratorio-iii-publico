CC = gcc
CFLAGS = -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror
LDFLAGS = -lpthread

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lrt
endif

all: checkin bodega

checkin: checkin.c cinta.h
	$(CC) $(CFLAGS) -o checkin checkin.c $(LDFLAGS)

bodega: bodega.c cinta.h
	$(CC) $(CFLAGS) -o bodega bodega.c $(LDFLAGS)

clean:
	rm -f checkin bodega
	rm -f /dev/shm/cinta_shm /dev/shm/sem.cinta_*

.PHONY: all clean
