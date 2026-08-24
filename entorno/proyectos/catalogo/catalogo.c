#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "catalogo.h"

int generar_id_aleatorio(void) {
    /* Genera un número aleatorio de 5 dígitos (10000 a 99999) */
    return (rand() % 90000) + 10000;
}

int contiene_subcadena_ci(const char *haystack, const char *needle) {
    size_t h_len;
    size_t n_len;
    size_t i;
    size_t j;

    if (haystack == NULL || needle == NULL) {
        return 0;
    }

    n_len = strlen(needle);
    if (n_len == 0) {
        return 1;
    }

    h_len = strlen(haystack);
    if (n_len > h_len) {
        return 0;
    }

    for (i = 0; i <= h_len - n_len; i++) {
        int match = 1;
        for (j = 0; j < n_len; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
                match = 0;
                break;
            }
        }
        if (match) {
            return 1;
        }
    }
    return 0;
}

int agregar_libro(const char *nombre_archivo, const char *titulo, const char *autor) {
    FILE *archivo;
    int id;

    if (nombre_archivo == NULL || titulo == NULL || autor == NULL) {
        fprintf(stderr, "Error: Parámetros nulos pasados a agregar_libro.\n");
        return 0;
    }

    if (strlen(titulo) == 0 || strlen(autor) == 0) {
        fprintf(stderr, "Error: El título y el autor no pueden estar vacíos.\n");
        return 0;
    }

    id = generar_id_aleatorio();

    archivo = fopen(nombre_archivo, "a");
    if (archivo == NULL) {
        fprintf(stderr, "Error: No se pudo abrir el archivo '%s' para escritura.\n", nombre_archivo);
        return 0;
    }

    fprintf(archivo, "%d;%s;%s\n", id, titulo, autor);
    fclose(archivo);

    printf("Libro agregado exitosamente:\n");
    printf("  ID:     %d\n", id);
    printf("  Título: %s\n", titulo);
    printf("  Autor:  %s\n", autor);

    return 1;
}

static int parsear_linea(const char *linea, Libro *libro) {
    char temp[MAX_BUFFER];
    char *token_id;
    char *token_titulo;
    char *token_autor;

    if (linea == NULL || libro == NULL) {
        return 0;
    }

    strncpy(temp, linea, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    /* Eliminar salto de línea */
    temp[strcspn(temp, "\r\n")] = '\0';

    if (strlen(temp) == 0) {
        return 0;
    }

    token_id = strtok(temp, ";");
    token_titulo = strtok(NULL, ";");
    token_autor = strtok(NULL, ";");

    if (token_id == NULL || token_titulo == NULL || token_autor == NULL) {
        return 0;
    }

    libro->id = atoi(token_id);
    strncpy(libro->titulo, token_titulo, MAX_TITULO - 1);
    libro->titulo[MAX_TITULO - 1] = '\0';

    strncpy(libro->autor, token_autor, MAX_AUTOR - 1);
    libro->autor[MAX_AUTOR - 1] = '\0';

    return 1;
}

int listar_libros(const char *nombre_archivo) {
    FILE *archivo;
    char buffer[MAX_BUFFER];
    Libro libro;
    int total_libros = 0;

    if (nombre_archivo == NULL) {
        fprintf(stderr, "Error: Nombre de archivo no válido.\n");
        return -1;
    }

    archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("El catálogo está vacío o el archivo '%s' aún no ha sido creado.\n", nombre_archivo);
        return 0;
    }

    printf("\n========================================================================================\n");
    printf("                               CATÁLOGO DE LIBROS                                      \n");
    printf("========================================================================================\n");
    printf("%-7s | %-45s | %-30s\n", "ID", "TÍTULO", "AUTOR");
    printf("--------+----------------------------------------------+--------------------------------\n");

    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        if (parsear_linea(buffer, &libro)) {
            printf("%-7d | %-45s | %-30s\n", libro.id, libro.titulo, libro.autor);
            total_libros++;
        }
    }

    fclose(archivo);

    printf("----------------------------------------------------------------------------------------\n");
    printf("Total de libros registrados: %d\n\n", total_libros);

    return total_libros;
}

int buscar_libros(const char *nombre_archivo, const char *texto_busqueda) {
    FILE *archivo;
    char buffer[MAX_BUFFER];
    Libro libro;
    int coincidencias = 0;

    if (nombre_archivo == NULL || texto_busqueda == NULL) {
        fprintf(stderr, "Error: Parámetros de búsqueda inválidos.\n");
        return -1;
    }

    if (strlen(texto_busqueda) == 0) {
        fprintf(stderr, "Error: El texto de búsqueda no puede estar vacío.\n");
        return -1;
    }

    archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("El catálogo está vacío o el archivo '%s' no existe.\n", nombre_archivo);
        printf("Coincidencias encontradas: 0\n");
        return 0;
    }

    printf("\nBuscando coincidencias para: \"%s\"\n", texto_busqueda);
    printf("----------------------------------------------------------------------------------------\n");
    printf("%-7s | %-45s | %-30s\n", "ID", "TÍTULO", "AUTOR");
    printf("--------+----------------------------------------------+--------------------------------\n");

    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        if (parsear_linea(buffer, &libro)) {
            if (contiene_subcadena_ci(libro.titulo, texto_busqueda) ||
                contiene_subcadena_ci(libro.autor, texto_busqueda)) {
                printf("%-7d | %-45s | %-30s\n", libro.id, libro.titulo, libro.autor);
                coincidencias++;
            }
        }
    }

    fclose(archivo);

    printf("----------------------------------------------------------------------------------------\n");
    printf("Coincidencias encontradas: %d\n\n", coincidencias);

    return coincidencias;
}
