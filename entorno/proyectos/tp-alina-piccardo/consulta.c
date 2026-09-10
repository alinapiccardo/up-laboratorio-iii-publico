/*
 * consulta.c - Cliente de Consulta de Estadísticas en Tiempo Real (SCADA Parte I)
 *
 * Asignatura: Laboratorio III
 * Estándar: C89 / ANSI C (-std=c89 -pedantic -Wall -Wextra -Werror)
 */

#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include "comun.h"

int main(void)
{
    int fd_shm;
    struct MemoriaCompartida *shm;
    sem_t *sem_mutex;
    int i;

    /* 1. Abrir la Memoria Compartida POSIX existente creada por el Colector */
    fd_shm = shm_open(NOMBRE_SHM, O_RDONLY, 0666);
    if (fd_shm == -1) {
        perror("Error: No se puede conectar a la Memoria Compartida. ¿Está corriendo el Colector?");
        return EXIT_FAILURE;
    }

    /* 2. Mapear el segmento en modo solo lectura (PROT_READ) */
    shm = (struct MemoriaCompartida *)mmap(NULL, sizeof(struct MemoriaCompartida),
                                           PROT_READ, MAP_SHARED, fd_shm, 0);
    if (shm == MAP_FAILED) {
        perror("Error en mmap solo lectura");
        close(fd_shm);
        return EXIT_FAILURE;
    }

    /* 3. Abrir el Semáforo POSIX con Nombre existente */
    sem_mutex = sem_open(NOMBRE_SEM, 0);
    if (sem_mutex == SEM_FAILED) {
        perror("Error: No se pudo acceder al semáforo de sincronización");
        munmap(shm, sizeof(struct MemoriaCompartida));
        close(fd_shm);
        return EXIT_FAILURE;
    }

    /* 4. Entrada a Sección Crítica: Lectura atómica protegida */
    if (sem_wait(sem_mutex) == -1) {
        perror("Error en sem_wait");
        sem_close(sem_mutex);
        munmap(shm, sizeof(struct MemoriaCompartida));
        close(fd_shm);
        return EXIT_FAILURE;
    }

    printf("\n=====================================================\n");
    printf(" MONITOR SCADA EN TIEMPO REAL - CLIENTE CONSULTA\n");
    printf("=====================================================\n");
    printf("Progreso General: %d / %d mediciones procesadas\n",
           shm->total_mediciones_recibidas, shm->total_esperado);
    printf("Sensores configurados: %d\n", shm->total_sensores);
    printf("-----------------------------------------------------\n");

    for (i = 0; i < shm->total_sensores; i++) {
        if (shm->sensores[i].cantidad > 0) {
            printf("Sensor %02d -> Muestras: %4d | Prom: %6.2f | Min: %6.2f | Max: %6.2f | Último: %6.2f\n",
                   shm->sensores[i].sensor_id,
                   shm->sensores[i].cantidad,
                   shm->sensores[i].suma / shm->sensores[i].cantidad,
                   shm->sensores[i].min,
                   shm->sensores[i].max,
                   shm->sensores[i].ultimo_valor);
        } else {
            printf("Sensor %02d -> Sin mediciones aún.\n", i + 1);
        }
    }
    printf("=====================================================\n\n");

    /* 5. Salida de Sección Crítica */
    if (sem_post(sem_mutex) == -1) {
        perror("Error en sem_post");
    }

    /* 6. Cierre local de recursos sin destruir el recurso global */
    sem_close(sem_mutex);
    munmap(shm, sizeof(struct MemoriaCompartida));
    close(fd_shm);

    return EXIT_SUCCESS;
}
