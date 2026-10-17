/*
 * Aplicacion cliente. Genera paginas a imprimir y las deposita en un
 * buffer circular en memoria compartida (el spooler), para que la
 * impresora las retire.
 *
 * Uso: ./cliente [cantidad]
 *   cantidad: numero de paginas a mandar (por defecto 20)
 *
 * Compilacion:
 *   gcc -std=c89 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -pedantic -Wall \
 *       -Werror cliente.c -o cliente -lpthread -lrt
 *
 * NOTA: Si el programa se interrumpe con Ctrl+C antes de terminar,
 *       los recursos IPC quedan en /dev/shm/ y hay que borrarlos:
 *       rm -f /dev/shm/spooler_impresion_buffer /dev/shm/sem.spooler_impresion_*
 *       (cliente.c ya hace esta limpieza el solo al arrancar, ver
 *       inicializar_recursos, asi que no deberias necesitarlo).
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

#include "compartido.h"

/* Intervalo entre paginas en microsegundos (50 ms) */
#define INTERVALO_US 50000

/* Cantidad por defecto de paginas a mandar */
#define CANTIDAD_DEFAULT 20

/* Variables globales */
static struct BufferCompartido *buffer = NULL;
static sem_t *sem_vacios = NULL;
static sem_t *sem_llenos = NULL;
static sem_t *sem_mutex = NULL;

/*
 * Genera una pagina simulada con tipo aleatorio.
 */
struct Pagina generar_pagina(int id)
{
    struct Pagina p;

    p.id = id;
    p.tipo = rand() % 2; /* 0 = blanco y negro, 1 = color */

    return p;
}

/*
 * Inicializa los recursos IPC: memoria compartida y semaforos.
 * el cliente es el "creador": hace O_CREAT y ftruncate.
 *
 * Antes de crear, borra cualquier recurso que haya quedado de una
 * corrida anterior interrumpida (por ejemplo con Ctrl+C). Si no se hace
 * esto, un semaforo puede quedar con un valor invalido (por ejemplo el
 * mutex en 0 porque el proceso murio dentro de la seccion critica), y
 * la proxima corrida se queda bloqueada para siempre en sem_wait.
 */
int inicializar_recursos(void)
{
    int shm_fd;

    /* limpieza de una corrida anterior (se ignora si no existian) */
    shm_unlink(SHM_NOMBRE);
    sem_unlink(SEM_VACIOS_NOMBRE);
    sem_unlink(SEM_LLENOS_NOMBRE);
    sem_unlink(SEM_MUTEX_NOMBRE);

    /* Crear la memoria compartida */
    shm_fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    /* Definir el tamanio */
    if (ftruncate(shm_fd, sizeof(struct BufferCompartido)) == -1) {
        perror("ftruncate");
        close(shm_fd);
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

    /* Inicializar el buffer completo (indices y acumuladores en cero) */
    memset(buffer, 0, sizeof(struct BufferCompartido));

    /* Crear los semaforos */
    sem_vacios = sem_open(SEM_VACIOS_NOMBRE, O_CREAT, 0600, BUFFER_CAPACIDAD);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open vacios");
        return -1;
    }

    sem_llenos = sem_open(SEM_LLENOS_NOMBRE, O_CREAT, 0600, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open llenos");
        return -1;
    }

    sem_mutex = sem_open(SEM_MUTEX_NOMBRE, O_CREAT, 0600, 1);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        return -1;
    }

    return 0;
}

/*
 * Libera los recursos IPC.
 * el cliente NO hace unlink: deja eso a la impresora (la ultima en irse).
 */
void liberar_recursos(void)
{
    if (buffer != NULL && buffer != MAP_FAILED) {
        munmap(buffer, sizeof(struct BufferCompartido));
    }
    if (sem_vacios != NULL && sem_vacios != SEM_FAILED) {
        sem_close(sem_vacios);
    }
    if (sem_llenos != NULL && sem_llenos != SEM_FAILED) {
        sem_close(sem_llenos);
    }
    if (sem_mutex != NULL && sem_mutex != SEM_FAILED) {
        sem_close(sem_mutex);
    }
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

    /* Semilla para numeros aleatorios */
    srand((unsigned int) time(NULL));

    printf("[Cliente] Iniciando...\n");
    printf("[Cliente] Mandando %d paginas a imprimir.\n\n", cantidad);

    /* Inicializar recursos IPC */
    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Cliente] Error inicializando recursos\n");
        return EXIT_FAILURE;
    }

    /* Loop principal: mandar 'cantidad' paginas */
    for (i = 0; i < cantidad; i++) {
        /* Generar la pagina */
        p = generar_pagina(i + 1);

        /* Esperar a que haya espacio en el spooler */
        sem_wait(sem_vacios);

        /* Tomar el mutex para acceder al buffer */
        sem_wait(sem_mutex);

        /* Escribir la pagina en la posicion cabeza */
        buffer->paginas[buffer->cabeza] = p;

        /* Avanzar cabeza con aritmetica modular */
        buffer->cabeza = (buffer->cabeza + 1) % BUFFER_CAPACIDAD;

        /* Liberar el mutex */
        sem_post(sem_mutex);

        /* Avisar que hay una pagina nueva */
        sem_post(sem_llenos);

        /* Mostrar lo que hicimos */
        printf("[Cliente] Pagina #%d: tipo=%s\n",
               p.id, (p.tipo == PAGINA_COLOR) ? "color" : "blanco y negro");

        /* Esperar antes de la proxima pagina */
        usleep(INTERVALO_US);
    }

    printf("\n[Cliente] Terminado. Se mandaron %d paginas.\n", cantidad);

    /* Limpiar (sin unlink, eso lo hace la impresora) */
    liberar_recursos();

    return EXIT_SUCCESS;
}
