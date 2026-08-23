#include <stdio.h>
#include <stdlib.h>

int main(void) {
  FILE *fp;
  fp = fopen("metrics.log", "r");

  if (fp == NULL) {
    perror("Error al abrir el archivo"); /* Imprime el mensaje que le pasamos*/
    exit(
        EXIT_FAILURE); /* Termina el programa con un codigo de error. usamos
                          EXIT_FAILURE porque funciona desde cualquier funcion*/
  }
  return 0;
}
