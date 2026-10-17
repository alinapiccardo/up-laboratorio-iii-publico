CC = gcc
CFLAGS = -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror
LIBS = -lpthread

# Soporte compatible para compilar tanto en Mac como en Linux con -lrt
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS += -lrt
endif

all: impresora app

impresora: impresora.c shared.h
	$(CC) $(CFLAGS) impresora.c -o impresora $(LIBS)

app: app.c shared.h
	$(CC) $(CFLAGS) app.c -o app $(LIBS)

clean:
	rm -f impresora app
