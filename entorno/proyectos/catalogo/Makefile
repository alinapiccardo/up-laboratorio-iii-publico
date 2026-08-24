# Makefile para el proyecto Catálogo de Biblioteca
# Asignatura: Laboratorio III

CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Wextra -Werror
TARGET = catalogo
OBJS = main.o catalogo.o

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c catalogo.h
	$(CC) $(CFLAGS) -c main.c

catalogo.o: catalogo.c catalogo.h
	$(CC) $(CFLAGS) -c catalogo.c

clean:
	rm -f $(OBJS) $(TARGET) catalogo.txt

test: $(TARGET)
	@echo "--- Ejecutando pruebas del catálogo ---"
	./$(TARGET) agregar "El Aleph" "Jorge Luis Borges"
	./$(TARGET) agregar "Cien años de soledad" "Gabriel García Márquez"
	./$(TARGET) agregar "Ficciones" "Jorge Luis Borges"
	./$(TARGET) agregar "Rayuela" "Julio Cortázar"
	@echo ""
	./$(TARGET) listar
	@echo ""
	./$(TARGET) buscar "Borges"
	@echo ""
	./$(TARGET) buscar "soledad"
