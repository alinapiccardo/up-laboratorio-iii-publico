/* si necesitamos que main reciba parámetros, la forma de llamarlo cambia.
int argc, char *argv[]
argc: argument count (es un int y es un contador de argumentos)
argv: argument vector (es un array de strings o sea un array de char*, donde
cada posicion contiene uno de los argumentos)
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  /*int cantidad;
  double precio;

  if (argc != 3) {
    fprintf(stderr, "Uso: %s <cantidad> <precio>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  cantidad = atoi(argv[1]);
  precio = atof(argv[2]);

  printf("Cantidad: %d\n", cantidad);
  printf("Precio: %.2f\n", precio);
  printf("Total: %.2f\n", cantidad * precio);*/

  /*----------- RAND -------- */
  /*Fórmula gral para obtener un número entre M y N (ambos incluidos) es:
    int n = rand() % (N-M+1) + M
    Siendo N el mayor valor que quiero obtener y M el menor.
  */

  /*----------- SRAND -------- */
  /*srand() permite "sembrar" la semilla del generador de números aleatorios.
  Para que cada ejecución dé una secuencia distinta, se suele usar la fecha
  actual como semilla.
  */
  srand(time(NULL));

  printf("%d\n", rand() % 100);
  printf("%d\n", rand() % 100);
  printf("%d\n", rand() % 100);

  return 0;
}