CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror
TARGET = pedidos
SRC = pedidos.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) pedidos.txt
