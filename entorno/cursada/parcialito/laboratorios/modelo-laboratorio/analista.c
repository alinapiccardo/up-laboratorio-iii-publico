/*
 * Analista del laboratorio. Lee muestras de un buffer circular en
 * memoria compartida, las procesa y lleva los acumuladores de control.
 *
 * Uso: ./analista [cantidad]
 *   cantidad: numero de muestras a procesar (por defecto 20, debe
 *             coincidir con la cantidad que se le paso a recepcion)
 *
 * Compilacion:
 *   gcc -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall \
 *       -Werror analista.c -o analista -lpthread -lrt
 *
 * NOTA: Si el programa se interrumpe con Ctrl+C antes de terminar,
 *       los recursos IPC quedan en /dev/shm/ y hay que borrarlos:
 *       rm -f /dev/shm/laboratorio_muestras_buffer /dev/shm/sem.laboratorio_muestras_*
 *       (la proxima corrida de recepcion los borra sola al arrancar).
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

/* Cantidad por defecto de muestras a procesar */
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
 * el analista NO crea: abre lo que recepcion ya creo. Si recepcion
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
            /* recepcion todavia no arranco, esperamos un poco */
            printf("[Analista] Esperando a que arranque recepcion...\n");
            sleep(1);
            intentos++;
        } else {
            perror("shm_open");
            return -1;
        }
    }

    if (shm_fd == -1) {
        fprintf(stderr, "[Analista] recepcion no esta corriendo.\n");
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
 * el analista ES el que hace unlink: es el "ultimo en irse".
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
    struct Muestra m;
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

    printf("[Analista] Iniciando...\n");
    printf("[Analista] Esperando %d muestras.\n\n", cantidad);

    /* Inicializar recursos IPC */
    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Analista] Error inicializando recursos\n");
        return EXIT_FAILURE;
    }

    printf("[Analista] Conectado al area de admision. Procesando...\n\n");

    /* Loop principal: procesar 'cantidad' muestras */
    for (i = 0; i < cantidad; i++) {
        /* Esperar a que haya una muestra en el area de admision */
        sem_wait(sem_llenos);

        /* Tomar el mutex para acceder al buffer */
        sem_wait(sem_mutex);

        /* Leer la muestra de la posicion cola */
        m = buffer->muestras[buffer->cola];

        /* Avanzar cola con aritmetica modular */
        buffer->cola = (buffer->cola + 1) % BUFFER_CAPACIDAD;

        /* Actualizar los acumuladores (dentro de la seccion critica,
         * para que no se pierda ningun incremento) */
        buffer->total = buffer->total + 1;
        if (m.tipo == MUESTRA_URGENTE) {
            buffer->urgentes = buffer->urgentes + 1;
        } else {
            buffer->rutinarias = buffer->rutinarias + 1;
        }

        /* Liberar el mutex */
        sem_post(sem_mutex);

        /* Avisar que hay un lugar libre */
        sem_post(sem_vacios);

        /* Procesar la muestra */
        printf("[Analista] #%d | Muestra %d | tipo=%s | total=%lu | "
               "rutinarias=%lu | urgentes=%lu\n", i + 1, m.id,
               (m.tipo == MUESTRA_URGENTE) ? "urgente" : "rutinaria",
               buffer->total, buffer->rutinarias, buffer->urgentes);

        usleep(150000);
    }

    printf("\n[Analista] Terminado. Se procesaron %d muestras.\n", cantidad);
    printf("[Analista] Totales -> muestras: %lu, rutinarias: %lu, "
           "urgentes: %lu\n", buffer->total, buffer->rutinarias,
           buffer->urgentes);

    /* Limpiar (incluyendo unlink) */
    liberar_recursos();

    printf("[Analista] Recursos eliminados del sistema.\n");

    return EXIT_SUCCESS;
}
