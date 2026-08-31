#include <stdio.h>
#include <stdlib.h>
#include "pedidos.h"

int main(void) {
    int numero;
    char descripcion[TAM_DESCRIPCION];
    char linea[TAM_LINEA];
    int total_procesados = 0;

    while (leer_proximo_pedido(&numero, descripcion)) {
        total_procesados++;
        formatear_linea(numero, descripcion, linea);
        printf("[Cocina] Preparando %s\n", linea);
    }

    printf("[Cocina] Total de pedidos preparados: %d\n", total_procesados);

    return EXIT_SUCCESS;
}
