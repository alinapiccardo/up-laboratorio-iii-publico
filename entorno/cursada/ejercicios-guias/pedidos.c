#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pedidos.h"

/*
 * Decisión sobre el mantenimiento del estado de lectura entre llamadas:
 * Se utiliza una variable estática 'static FILE *f_lectura = NULL;' interna a este módulo.
 * - En la primera llamada a 'leer_proximo_pedido', si 'f_lectura' es NULL, se abre el archivo
 *   "pedidos.txt" en modo lectura ("r").
 * - En llamadas subsiguientes, se continúa leyendo línea a línea manteniendo el puntero abierto.
 * - Cuando 'fgets' llega al final del archivo (EOF) o sufre un error, el archivo se cierra
 *   con 'fclose' y 'f_lectura' se vuelve a setear en NULL.
 * De esta manera no se requiere que el usuario de la librería mantenga el puntero FILE *,
 * encapsulando el estado dentro de la librería.
 */
static FILE *f_lectura = NULL;

void formatear_linea(int numero, const char *descripcion, char *destino) {
    if (destino != NULL && descripcion != NULL) {
        sprintf(destino, "PEDIDO-%03d: %s", numero, descripcion);
    }
}

int registrar_pedido(int numero, const char *descripcion) {
    FILE *f_escritura;
    char linea[TAM_LINEA];

    if (descripcion == NULL) {
        return 0;
    }

    f_escritura = fopen(NOMBRE_ARCHIVO_PEDIDOS, "a");
    if (f_escritura == NULL) {
        perror("Error al abrir pedidos.txt en modo append");
        return 0;
    }

    formatear_linea(numero, descripcion, linea);
    fprintf(f_escritura, "%s\n", linea);

    if (fclose(f_escritura) == EOF) {
        perror("Error al cerrar pedidos.txt despues de registrar");
        return 0;
    }

    return 1;
}

int leer_proximo_pedido(int *numero, char *descripcion) {
    char buffer[TAM_LINEA];

    if (numero == NULL || descripcion == NULL) {
        return 0;
    }

    if (f_lectura == NULL) {
        f_lectura = fopen(NOMBRE_ARCHIVO_PEDIDOS, "r");
        if (f_lectura == NULL) {
            return 0;
        }
    }

    while (fgets(buffer, sizeof(buffer), f_lectura) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == 0) {
            continue;
        }

        if (sscanf(buffer, "PEDIDO-%d: %[^\n]", numero, descripcion) == 2) {
            return 1;
        }
    }

    fclose(f_lectura);
    f_lectura = NULL;
    return 0;
}

int contar_pedidos_existentes(void) {
    FILE *f;
    char buffer[TAM_LINEA];
    int cantidad = 0;
    int num_temp;
    char desc_temp[TAM_DESCRIPCION];

    f = fopen(NOMBRE_ARCHIVO_PEDIDOS, "r");
    if (f == NULL) {
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) > 0 && sscanf(buffer, "PEDIDO-%d: %[^\n]", &num_temp, desc_temp) == 2) {
            cantidad++;
        }
    }

    fclose(f);
    return cantidad;
}
