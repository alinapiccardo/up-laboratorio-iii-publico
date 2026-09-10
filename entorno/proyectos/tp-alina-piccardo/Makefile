# Makefile para el Trabajo Final Integrador - Parte I (SCADA)
# Asignatura: Laboratorio III
# Estándar: C89 / ANSI C estricto

CC = gcc
CFLAGS = -std=c89 -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -pedantic -Wall -Wextra -Werror
LIBS = -lpthread -lrt

TARGETS = colector consulta

.PHONY: all clean ipc_clean test help

all: $(TARGETS)

colector: colector.c comun.h
	$(CC) $(CFLAGS) colector.c -o colector $(LIBS)

consulta: consulta.c comun.h
	$(CC) $(CFLAGS) consulta.c -o consulta $(LIBS)

clean:
	rm -f $(TARGETS) *.o
	@echo "[Makefile] Binarios y objetos limpios."

ipc_clean:
	rm -f /dev/shm/shm_scada_stats /dev/shm/sem_scada_mutex /dev/mqueue/mq_scada_mediciones
	@echo "[Makefile] Recursos IPC en /dev/shm/ y /dev/mqueue/ eliminados."

test: all
	@echo "--- Ejecutando prueba automatizada del sistema SCADA ---"
	./colector 4 25 2

help:
	@echo "Opciones de compilación disponibles:"
	@echo "  make          : Compila colector y consulta con reglas C89 estrictas."
	@echo "  make clean    : Elimina los binarios ejecutables."
	@echo "  make ipc_clean: Borra los recursos IPC residuales del sistema."
	@echo "  make test     : Ejecuta una simulación de prueba con 4 sensores y 25 mediciones c/u."
