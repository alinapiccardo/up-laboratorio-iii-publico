#include "cadenas.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  char texto[] = "cambie";
  char copia[100];

  strcpy(copia, texto);

  printf("Texto: \"%s\"\n", texto);
  printf("Vocales: %d\n", contar_vocales(texto));
  printf("Palabras: %d\n", contar_palabras(texto));
  printf("Es palindromo: %s\n", es_palindromo(texto) ? "si" : "no");

  a_mayusculas(copia);
  printf("En mayusculas: \"%s\"\n", copia);

  invertir_cadena(copia);
  printf("Invertido: \"%s\"\n", copia);

  return 0;
}
