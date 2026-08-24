#ifndef CATALOGO_H
#define CATALOGO_H

#define MAX_TITULO 150
#define MAX_AUTOR 100
#define MAX_BUFFER 500
#define NOMBRE_ARCHIVO_DEF "catalogo.txt"

typedef struct {
    int id;
    char titulo[MAX_TITULO];
    char autor[MAX_AUTOR];
} Libro;

/*
 * Genera un ID entero aleatorio positivo de 5 dígitos (entre 10000 y 99999).
 */
int generar_id_aleatorio(void);

/*
 * Agrega un nuevo libro al archivo especificado.
 * Devuelve 1 en caso de éxito o 0 en caso de error.
 */
int agregar_libro(const char *nombre_archivo, const char *titulo, const char *autor);

/*
 * Lista todos los libros almacenados en el archivo especificado.
 * Devuelve el total de libros listados o -1 en caso de error.
 */
int listar_libros(const char *nombre_archivo);

/*
 * Busca e imprime libros cuyos títulos o autores contengan la subcadena especificada
 * (insensible a mayúsculas/minúsculas).
 * Al final muestra y devuelve la cantidad de coincidencias encontradas.
 */
int buscar_libros(const char *nombre_archivo, const char *texto_busqueda);

/*
 * Función auxiliar para verificar si una cadena contiene a otra como subcadena
 * de manera insensible a mayúsculas/minúsculas.
 */
int contiene_subcadena_ci(const char *haystack, const char *needle);

#endif /* CATALOGO_H */
