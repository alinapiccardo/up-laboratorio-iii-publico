#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CANTIDAD_DEFAULT 10
#define DESDE_DEFAULT 0
#define HASTA_DEFAULT 10

int main(int argc, char *argv[]) {
  int cantidad = CANTIDAD_DEFAULT;
  int desde = DESDE_DEFAULT;
  int hasta = HASTA_DEFAULT;
  int i;
  int numero;

  if (argc != 1 && argc != 4) {
    fprintf(stderr, "Uso: %s <cantidad> <desde> <hasta>\n", argv[0]);
    fprintf(stderr, "Sin argumentos usa los valores por defecto .\n");
    exit(EXIT_FAILURE);
  }

  if (argc == 4) {
    cantidad = atoi(argv[1]);
    desde = atoi(argv[2]);
    hasta = atoi(argv[3]);
  }

  if (cantidad <= 0 || hasta < desde) {
    fprintf(stderr, "Error en los argumentos.\n");
    exit(EXIT_FAILURE);
  }

  srand(time(NULL));

  printf("Generando %d numeros entre %d y %d:\n", cantidad, desde, hasta);
  for (i = 0; i < cantidad; i++) {
    numero = rand() % (hasta - desde + 1) + desde;
    printf("%d ", numero);
  }
  printf("\n");
  return 0;
}