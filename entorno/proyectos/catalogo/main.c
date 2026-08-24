#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catalogo.h"

static void mostrar_ayuda(const char *nombre_programa) {
    printf("Uso: %s [comando] [argumentos]\n\n", nombre_programa);
    printf("Comandos disponibles:\n");
    printf("  agregar <titulo> <autor>    Agrega un nuevo libro con un ID aleatorio.\n");
    printf("  listar                      Muestra la lista de todos los libros registrados.\n");
    printf("  buscar <texto>              Busca libros por subcadena en título o autor.\n");
    printf("  ayuda, -h, --help           Muestra esta información de ayuda.\n\n");
    printf("Si se ejecuta sin argumentos, el programa iniciará el modo interactivo por consola.\n");
}

static void leer_cadena_consola(const char *prompt, char *destino, size_t tamano) {
    printf("%s", prompt);
    if (fgets(destino, (int)tamano, stdin) != NULL) {
        destino[strcspn(destino, "\r\n")] = '\0';
    } else {
        destino[0] = '\0';
    }
}

static void menu_interactivo(void) {
    int opcion = 0;
    char buffer_opcion[32];
    char titulo[MAX_TITULO];
    char autor[MAX_AUTOR];
    char texto_busqueda[MAX_TITULO];

    do {
        printf("=======================================\n");
        printf("      SISTEMA DE CATÁLOGO DE LIBROS    \n");
        printf("=======================================\n");
        printf("1. Agregar un libro\n");
        printf("2. Listar libros\n");
        printf("3. Buscar libros\n");
        printf("4. Salir\n");
        printf("---------------------------------------\n");
        
        leer_cadena_consola("Seleccione una opción [1-4]: ", buffer_opcion, sizeof(buffer_opcion));
        opcion = atoi(buffer_opcion);

        switch (opcion) {
            case 1:
                printf("\n--- Agregar un libro ---\n");
                leer_cadena_consola("Ingrese el título del libro: ", titulo, sizeof(titulo));
                leer_cadena_consola("Ingrese el autor del libro:  ", autor, sizeof(autor));
                if (strlen(titulo) == 0 || strlen(autor) == 0) {
                    printf("Error: El título y el autor no pueden estar vacíos.\n\n");
                } else {
                    agregar_libro(NOMBRE_ARCHIVO_DEF, titulo, autor);
                    printf("\n");
                }
                break;

            case 2:
                listar_libros(NOMBRE_ARCHIVO_DEF);
                break;

            case 3:
                printf("\n--- Buscar libros ---\n");
                leer_cadena_consola("Ingrese el texto a buscar (título o autor): ", texto_busqueda, sizeof(texto_busqueda));
                if (strlen(texto_busqueda) == 0) {
                    printf("Error: El texto de búsqueda no puede estar vacío.\n\n");
                } else {
                    buscar_libros(NOMBRE_ARCHIVO_DEF, texto_busqueda);
                }
                break;

            case 4:
                printf("Saliendo del programa... ¡Hasta luego!\n");
                break;

            default:
                printf("Opción no válida. Por favor, intente nuevamente.\n\n");
                break;
        }
    } while (opcion != 4);
}

int main(int argc, char *argv[]) {
    const char *comando;

    /* Inicializar la semilla para números aleatorios combinando time y clock */
    srand((unsigned int)time(NULL) ^ (unsigned int)clock());

    /* Modo línea de comandos (argc / argv) */
    if (argc > 1) {
        comando = argv[1];

        if (strcmp(comando, "agregar") == 0 || strcmp(comando, "-a") == 0 || strcmp(comando, "--agregar") == 0) {
            if (argc < 4) {
                fprintf(stderr, "Error: Argumentos insuficientes para agregar un libro.\n");
                fprintf(stderr, "Uso: %s agregar \"<titulo>\" \"<autor>\"\n", argv[0]);
                return EXIT_FAILURE;
            }
            if (!agregar_libro(NOMBRE_ARCHIVO_DEF, argv[2], argv[3])) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }

        if (strcmp(comando, "listar") == 0 || strcmp(comando, "-l") == 0 || strcmp(comando, "--listar") == 0) {
            if (listar_libros(NOMBRE_ARCHIVO_DEF) < 0) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }

        if (strcmp(comando, "buscar") == 0 || strcmp(comando, "-b") == 0 || strcmp(comando, "--buscar") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Error: Argumentos insuficientes para buscar libros.\n");
                fprintf(stderr, "Uso: %s buscar \"<texto>\"\n", argv[0]);
                return EXIT_FAILURE;
            }
            if (buscar_libros(NOMBRE_ARCHIVO_DEF, argv[2]) < 0) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }

        if (strcmp(comando, "ayuda") == 0 || strcmp(comando, "-h") == 0 || strcmp(comando, "--help") == 0) {
            mostrar_ayuda(argv[0]);
            return EXIT_SUCCESS;
        }

        fprintf(stderr, "Error: Comando '%s' desconocido.\n\n", comando);
        mostrar_ayuda(argv[0]);
        return EXIT_FAILURE;
    }

    /* Modo interactivo por consola */
    menu_interactivo();

    return EXIT_SUCCESS;
}
