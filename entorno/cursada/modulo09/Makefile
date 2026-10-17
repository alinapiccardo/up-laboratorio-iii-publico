CC      = gcc
CFLAGS  = -std=c89 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror
LDLIBS  = -lpthread -lrt

PROG    = 09_iot_sistema
OBJS    = 09_iot_main.o 09_iot_sensor.o 09_iot_procesador.o 09_iot_util.o

all: $(PROG)

# Enlazado: une los .o y agrega las bibliotecas al final
$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LDLIBS)

# Compilacion: cada .c se compila por separado a su .o
%.o: %.c 09_iot.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(PROG)
	./$(PROG) 3

clean:
	rm -f $(PROG) $(OBJS)

.PHONY: all run clean