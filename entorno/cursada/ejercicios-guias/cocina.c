#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "pedidos.h"

static void manejar_sigint(int sig) {
    (void)sig;
    printf("\n[Cocina] Interrumpido. Limpiando semaforos...\n");
    fflush(stdout);
    destruir_semaforos();
    exit(EXIT_SUCCESS);
}

int main(void) {
    int numero;
    char descripcion[TAM_DESCRIPCION];
    char linea[TAM_LINEA];
    int total_procesados = 0;

    signal(SIGINT, manejar_sigint);
    signal(SIGTERM, manejar_sigint);

    if (!inicializar_semaforos()) {
        fprintf(stderr, "Error al inicializar semaforos en cocina.\n");
        return EXIT_FAILURE;
    }

    printf("[Cocina] Esperando pedidos...\n");
    fflush(stdout);

    while (leer_proximo_pedido(&numero, descripcion)) {
        total_procesados++;
        formatear_linea(numero, descripcion, linea);
        printf("[Cocina] Preparando %s (Total: %d)\n", linea, total_procesados);
        fflush(stdout);
    }

    printf("[Cocina] Total de pedidos preparados: %d\n", total_procesados);
    fflush(stdout);
    cerrar_semaforos();

    return EXIT_SUCCESS;
}


