#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "pedidos.h"

static void manejar_sigint(int sig) {
    (void)sig;
    destruir_semaforos();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int cantidad_pedidos;
    int i;
    int inicio_numero;
    int numero_actual;
    const char *desc_azar;
    char linea[TAM_LINEA];
    const char *descripciones[] = {
        "Hamburguesa con papas",
        "Pizza grande",
        "Ensalada Cesar",
        "Pasta Alfredo",
        "Sushi mixto"
    };
    int total_descripciones;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <cantidad>\n", argv[0]);
        return EXIT_FAILURE;
    }

    cantidad_pedidos = atoi(argv[1]);
    if (cantidad_pedidos <= 0) {
        fprintf(stderr, "Error: la cantidad de pedidos debe ser mayor a 0.\n");
        return EXIT_FAILURE;
    }

    signal(SIGINT, manejar_sigint);
    signal(SIGTERM, manejar_sigint);

    if (!inicializar_semaforos()) {
        fprintf(stderr, "Error al inicializar semaforos en recepcion.\n");
        return EXIT_FAILURE;
    }

    total_descripciones = (int)(sizeof(descripciones) / sizeof(descripciones[0]));
    srand((unsigned int)time(NULL));

    inicio_numero = contar_pedidos_existentes() + 1;

    for (i = 0; i < cantidad_pedidos; i++) {
        desc_azar = descripciones[rand() % total_descripciones];
        numero_actual = inicio_numero + i;

        if (!registrar_pedido(numero_actual, desc_azar)) {
            fprintf(stderr, "Error al registrar el pedido %d.\n", numero_actual);
            cerrar_semaforos();
            return EXIT_FAILURE;
        }

        formatear_linea(numero_actual, desc_azar, linea);
        printf("[Recepcion] Registrando %s\n", linea);
    }

    cerrar_semaforos();
    return EXIT_SUCCESS;
}

