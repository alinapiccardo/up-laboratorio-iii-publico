#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  char clave[] = "secreto";
  char intento[50];

  printf("Ingresa la clave: ");
  scanf("%49s", intento);

  if (strcmp(clave, intento) == 0) {
    printf("Clave correcta\n ");
  } else {
    printf("Clave incorrecta\n");
  }

  return 0;
}