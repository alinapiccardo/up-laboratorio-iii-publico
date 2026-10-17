/*
 * Impresora. Lee paginas de un buffer circular en memoria compartida
 * (el spooler), las "imprime" y lleva los acumuladores de control.
 *
 * Uso: ./impresora [cantidad]
 *   cantidad: numero de paginas a imprimir (por defecto 20, debe
 *             coincidir con la cantidad que se le paso a cliente)
 *
 * Compilacion:
 *   gcc -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall \
 *       -Werror impresora.c -o impresora -lpthread -lrt
 *
 * NOTA: Si el programa se interrumpe con Ctrl+C antes de terminar,
 *       los recursos IPC quedan en /dev/shm/ y hay que borrarlos:
 *       rm -f /dev/shm/spooler_impresion_buffer /dev/shm/sem.spooler_impresion_*
 *       (la proxima corrida de cliente los borra sola al arrancar).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>

#include "compartido.h"

/* Cantidad por defecto de paginas a imprimir */
#define CANTIDAD_DEFAULT 20

/* Cuantas veces reintenta conectarse antes de rendirse */
#define REINTENTOS_MAX 10

/* Variables globales */
static struct BufferCompartido *buffer = NULL;
static sem_t *sem_vacios = NULL;
static sem_t *sem_llenos = NULL;
static sem_t *sem_mutex = NULL;

/*
 * Inicializa los recursos IPC.
 * la impresora NO crea: abre lo que el cliente ya creo. Si el cliente
 * todavia no arranco, reintenta unas cuantas veces en vez de fallar de una.
 */
int inicializar_recursos(void)
{
    int shm_fd;
    int intentos;

    /* Abrir la memoria compartida (sin O_CREAT: debe existir) */
    intentos = 0;
    shm_fd = -1;
    while (intentos < REINTENTOS_MAX) {
        shm_fd = shm_open(SHM_NOMBRE, O_RDWR, 0);
        if (shm_fd != -1) {
            break;
        }
        if (errno == ENOENT) {
            /* el cliente todavia no arranco, esperamos un poco */
            printf("[Impresora] Esperando a que arranque el cliente...\n");
            sleep(1);
            intentos++;
        } else {
            perror("shm_open");
            return -1;
        }
    }

    if (shm_fd == -1) {
        fprintf(stderr, "[Impresora] el cliente no esta corriendo.\n");
        return -1;
    }

    /* Mapear en el espacio de direcciones */
    buffer = mmap(NULL, sizeof(struct BufferCompartido),
                  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return -1;
    }

    /* Ya no necesitamos el fd */
    close(shm_fd);

    /* Abrir los semaforos (sin O_CREAT) */
    sem_vacios = sem_open(SEM_VACIOS_NOMBRE, 0);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open vacios");
        return -1;
    }

    sem_llenos = sem_open(SEM_LLENOS_NOMBRE, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open llenos");
        return -1;
    }

    sem_mutex = sem_open(SEM_MUTEX_NOMBRE, 0);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        return -1;
    }

    return 0;
}

/*
 * Libera los recursos IPC.
 * la impresora ES la que hace unlink: es la "ultima en irse".
 */
void liberar_recursos(void)
{
    if (buffer != NULL && buffer != MAP_FAILED) {
        munmap(buffer, sizeof(struct BufferCompartido));
    }
    if (sem_vacios != NULL && sem_vacios != SEM_FAILED) {
        sem_close(sem_vacios);
        sem_unlink(SEM_VACIOS_NOMBRE);
    }
    if (sem_llenos != NULL && sem_llenos != SEM_FAILED) {
        sem_close(sem_llenos);
        sem_unlink(SEM_LLENOS_NOMBRE);
    }
    if (sem_mutex != NULL && sem_mutex != SEM_FAILED) {
        sem_close(sem_mutex);
        sem_unlink(SEM_MUTEX_NOMBRE);
    }

    /* Eliminar la memoria compartida */
    shm_unlink(SHM_NOMBRE);
}

int main(int argc, char *argv[])
{
    struct Pagina p;
    int cantidad;
    int i;

    /* Parsear argumento */
    if (argc > 1) {
        cantidad = atoi(argv[1]);
        if (cantidad <= 0) {
            cantidad = CANTIDAD_DEFAULT;
        }
    } else {
        cantidad = CANTIDAD_DEFAULT;
    }

    printf("[Impresora] Iniciando...\n");
    printf("[Impresora] Esperando %d paginas.\n\n", cantidad);

    /* Inicializar recursos IPC */
    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Impresora] Error inicializando recursos\n");
        return EXIT_FAILURE;
    }

    printf("[Impresora] Conectada al spooler. Imprimiendo...\n\n");

    /* Loop principal: imprimir 'cantidad' paginas */
    for (i = 0; i < cantidad; i++) {
        /* Esperar a que haya una pagina en el spooler */
        sem_wait(sem_llenos);

        /* Tomar el mutex para acceder al buffer */
        sem_wait(sem_mutex);

        /* Leer la pagina de la posicion cola */
        p = buffer->paginas[buffer->cola];

        /* Avanzar cola con aritmetica modular */
        buffer->cola = (buffer->cola + 1) % BUFFER_CAPACIDAD;

        /* Actualizar los acumuladores (dentro de la seccion critica,
         * para que no se pierda ningun incremento) */
        buffer->total = buffer->total + 1;
        if (p.tipo == PAGINA_COLOR) {
            buffer->color = buffer->color + 1;
        } else {
            buffer->bn = buffer->bn + 1;
        }

        /* Liberar el mutex */
        sem_post(sem_mutex);

        /* Avisar que hay un lugar libre */
        sem_post(sem_vacios);

        /* Procesar la pagina (imprimirla, simulado con una pausa) */
        printf("[Impresora] #%d | Pagina %d | tipo=%s | total=%lu | "
               "bn=%lu | color=%lu\n", i + 1, p.id,
               (p.tipo == PAGINA_COLOR) ? "color" : "blanco y negro",
               buffer->total, buffer->bn, buffer->color);

        usleep(200000);
    }

    printf("\n[Impresora] Terminado. Se imprimieron %d paginas.\n", cantidad);
    printf("[Impresora] Totales -> paginas: %lu, bn: %lu, color: %lu\n",
           buffer->total, buffer->bn, buffer->color);

    /* Limpiar (incluyendo unlink) */
    liberar_recursos();

    printf("[Impresora] Recursos eliminados del sistema.\n");

    return EXIT_SUCCESS;
}
