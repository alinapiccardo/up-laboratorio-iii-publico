#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANTIDAD_PEDIDOS 10
#define NOMBRE_ARCHIVO "pedidos.txt"
#define TAM_BUFFER 256

int recibir_pedidos(const char *nombre_archivo, int cantidad) {
  FILE *f_escritura;
  int i;
  const char *descripcion;
  const char *menu[] = {"Hamburguesa con papas",
                        "Pizza grande de muzzarella",
                        "Ensalada Caesar",
                        "Empanadas fritas",
                        "Milanesa a la napolitana con pure",
                        "Ensalada de la Casa",
                        "Fajitas de carne asada",
                        "Lomito completo",
                        "Fideos a la bolognesa",
                        "Flan casero con dulce de leche"};
  int total_menu = sizeof(menu) / sizeof(menu[0]);

  printf("=====================================================\n");
  printf("1. RECEPCIÓN DE PEDIDOS\n");
  printf("=====================================================\n");

  f_escritura = fopen(nombre_archivo, "w");
  if (f_escritura == NULL) {
    perror("Error al abrir el archivo .txt para escritura");
    return 0;
  }

  for (i = 1; i <= cantidad; i++) {
    descripcion = menu[(i - 1) % total_menu];
    fprintf(f_escritura, "PEDIDO-%03d: %s\n", i, descripcion);
    printf("[RECEPCIÓN] Registrado -> PEDIDO-%03d: %s\n", i, descripcion);
  }

  if (fclose(f_escritura) == EOF) {
    perror("Error al cerrar pedidos.txt despues de escribir");
    return 0;
  }

  printf("--> Éxito: Se registraron %d pedidos en '%s'.\n\n", cantidad,
         nombre_archivo);
  return 1;
}

int preparar_pedidos(const char *nombre_archivo) {
  FILE *f_lectura;
  char buffer[TAM_BUFFER];
  int contador = 0;

  printf("=====================================================\n");
  printf("2. PREPARACIÓN DE PEDIDOS EN COCINA\n");
  printf("=====================================================\n");

  f_lectura = fopen(nombre_archivo, "r");
  if (f_lectura == NULL) {
    perror("Error al abrir pedidos.txt para lectura");
    return 0;
  }

  while (fgets(buffer, sizeof(buffer), f_lectura) != NULL) {
    /* Eliminar salto de línea al final del buffer */
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strlen(buffer) > 0) {
      contador++;
      printf("[COCINA] Preparando y listo -> %s\n", buffer);
    }
  }

  if (!feof(f_lectura)) {
    fprintf(stderr, "Error: el archivo tiene un formato invalido\n");
    fclose(f_lectura);
    return 0;
  }

  if (fclose(f_lectura) == EOF) {
    perror("Error al cerrar pedidos.txt despues de leer");
    return 0;
  }

  printf("--> Éxito: Se procesaron y prepararon %d pedidos desde '%s'.\n\n",
         contador, nombre_archivo);
  return contador;
}

int main(void) {
  printf("-----------------------------------------------------\n");
  printf("   SISTEMA DE GESTIÓN DE PEDIDOS DE RESTAURANTE      \n");
  printf("-----------------------------------------------------\n\n");

  /* 1. Recepción de pedidos */
  if (!recibir_pedidos(NOMBRE_ARCHIVO, CANTIDAD_PEDIDOS)) {
    fprintf(stderr, "Error durante la fase de recepción de pedidos.\n");
    return EXIT_FAILURE;
  }

  /* 2. Preparación de pedidos en cocina */
  if (!preparar_pedidos(NOMBRE_ARCHIVO)) {
    fprintf(stderr, "Error durante la fase de preparación de pedidos.\n");
    return EXIT_FAILURE;
  }

  printf("Proceso completado exitosamente.\n");
  return EXIT_SUCCESS;
}
