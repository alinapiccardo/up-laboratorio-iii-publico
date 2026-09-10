/*
 * colector.c - Proceso Colector y Administrador de Sensores (SCADA Parte I)
 *
 * Asignatura: Laboratorio III
 * Estándar: C89 / ANSI C (-std=c89 -pedantic -Wall -Wextra -Werror)
 */

#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <mqueue.h>
#include <pthread.h>

#include "comun.h"

/* Intervalo de simulación de lecturas de sensor en microsegundos (50ms) */
#define INTERVALO_SENSOR_US 50000

/* Estructura de argumentos pasados a cada worker thread */
struct WorkerArg {
    int id;
    mqd_t mq;
    struct MemoriaCompartida *shm;
    sem_t *sem_mutex;
};

/* Prototipos de funciones */
static void ejecutar_sensor(int sensor_id, int cant_mediciones);
static void *worker_function(void *arg);
static void limpiar_recursos_ipc(mqd_t mq, sem_t *sem_mutex, struct MemoriaCompartida *shm, int fd_shm);

/*
 * Función ejecutada por cada proceso sensor hijo (creado con fork)
 */
static void ejecutar_sensor(int sensor_id, int cant_mediciones)
{
    mqd_t mq_sensor;
    struct Medicion m;
    int i;

    /* Semilla única por sensor combinando tiempo actual y PID */
    srand((unsigned int)(time(NULL) ^ getpid()));

    mq_sensor = mq_open(NOMBRE_COLA, O_WRONLY);
    if (mq_sensor == (mqd_t)-1) {
        perror("Error en sensor al abrir mq_open");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < cant_mediciones; i++) {
        m.sensor_id = sensor_id;
        /* Valor de medición simulado (ej. temperatura entre 15.0 y 35.0 °C) */
        m.valor = 15.0 + ((double)rand() / RAND_MAX) * 20.0;
        m.timestamp = (long)time(NULL);

        if (mq_send(mq_sensor, (const char *)&m, sizeof(m), 0) == -1) {
            perror("Error en sensor al ejecutar mq_send");
        }

        /* Espera simulada entre mediciones */
        usleep(INTERVALO_SENSOR_US);
    }

    mq_close(mq_sensor);
    exit(EXIT_SUCCESS);
}

/*
 * Función ejecutada por cada Worker Thread en el Colector
 */
static void *worker_function(void *arg)
{
    struct WorkerArg *warg;
    struct Medicion m;
    struct StatsSensor *st;
    ssize_t bytes_read;
    int sensor_idx;

    warg = (struct WorkerArg *)arg;

    while (1) {
        bytes_read = mq_receive(warg->mq, (char *)&m, sizeof(m), NULL);
        if (bytes_read < 0) {
            perror("Error en worker thread al ejecutar mq_receive");
            break;
        }

        /* Centinela de terminación: si sensor_id == -1, terminar e informar a los demás */
        if (m.sensor_id == -1) {
            mq_send(warg->mq, (const char *)&m, sizeof(m), 0);
            break;
        }

        /* Sección Crítica: Actualización de Estadísticas en Memoria Compartida */
        if (sem_wait(warg->sem_mutex) == -1) {
            perror("Error en sem_wait");
            break;
        }

        sensor_idx = m.sensor_id - 1;
        if (sensor_idx >= 0 && sensor_idx < MAX_SENSORES) {
            st = &warg->shm->sensores[sensor_idx];

            if (st->cantidad == 0) {
                st->sensor_id = m.sensor_id;
                st->min = m.valor;
                st->max = m.valor;
            } else {
                if (m.valor < st->min) {
                    st->min = m.valor;
                }
                if (m.valor > st->max) {
                    st->max = m.valor;
                }
            }

            st->cantidad++;
            st->suma += m.valor;
            st->ultimo_valor = m.valor;

            warg->shm->total_mediciones_recibidas++;

            /* Si alcanzamos el total esperado de mediciones, enviamos centinela a la cola */
            if (warg->shm->total_mediciones_recibidas >= warg->shm->total_esperado) {
                struct Medicion centinela;
                centinela.sensor_id = -1;
                centinela.valor = 0.0;
                centinela.timestamp = 0;
                mq_send(warg->mq, (const char *)&centinela, sizeof(centinela), 0);
            }
        }

        if (sem_post(warg->sem_mutex) == -1) {
            perror("Error en sem_post");
            break;
        }
    }

    return NULL;
}

/*
 * Cierre y liberación ordenada de recursos IPC
 */
static void limpiar_recursos_ipc(mqd_t mq, sem_t *sem_mutex, struct MemoriaCompartida *shm, int fd_shm)
{
    if (mq != (mqd_t)-1) {
        mq_close(mq);
        mq_unlink(NOMBRE_COLA);
    }

    if (sem_mutex != SEM_FAILED && sem_mutex != NULL) {
        sem_close(sem_mutex);
        sem_unlink(NOMBRE_SEM);
    }

    if (shm != MAP_FAILED && shm != NULL) {
        munmap(shm, sizeof(struct MemoriaCompartida));
        shm_unlink(NOMBRE_SHM);
    }

    if (fd_shm != -1) {
        close(fd_shm);
    }
}

int main(int argc, char *argv[])
{
    int num_sensores;
    int mediciones_por_sensor;
    int num_workers;
    int i;

    int fd_shm;
    struct MemoriaCompartida *shm;
    sem_t *sem_mutex;
    mqd_t mq;
    struct mq_attr attr;

    pthread_t workers[MAX_SENSORES];
    struct WorkerArg args[MAX_SENSORES];
    pid_t pids[MAX_SENSORES];

    fd_shm = -1;
    shm = MAP_FAILED;
    sem_mutex = SEM_FAILED;
    mq = (mqd_t)-1;

    /* 1. Validación de argumentos por línea de comandos (argv) */
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <cant_sensores> <mediciones_por_sensor> [num_workers]\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_sensores = atoi(argv[1]);
    mediciones_por_sensor = atoi(argv[2]);
    num_workers = (argc >= 4) ? atoi(argv[3]) : num_sensores;

    if (num_sensores <= 0 || num_sensores > MAX_SENSORES) {
        fprintf(stderr, "Error: la cantidad de sensores debe estar entre 1 y %d.\n", MAX_SENSORES);
        return EXIT_FAILURE;
    }

    if (mediciones_por_sensor <= 0) {
        fprintf(stderr, "Error: las mediciones por sensor deben ser mayores a 0.\n");
        return EXIT_FAILURE;
    }

    if (num_workers <= 0 || num_workers > MAX_SENSORES) {
        num_workers = num_sensores;
    }

    printf("=====================================================\n");
    printf(" INICIANDO COLECTOR SCADA (Parte I)\n");
    printf(" Sensores: %d | Mediciones/Sensor: %d | Total Esperado: %d\n",
           num_sensores, mediciones_por_sensor, num_sensores * mediciones_por_sensor);
    printf(" Worker Threads: %d\n", num_workers);
    printf("=====================================================\n");

    /* Limpieza preventiva de recursos IPC huérfanos preexistentes */
    mq_unlink(NOMBRE_COLA);
    shm_unlink(NOMBRE_SHM);
    sem_unlink(NOMBRE_SEM);

    /* 2. Inicialización de Memoria Compartida POSIX */
    fd_shm = shm_open(NOMBRE_SHM, O_CREAT | O_RDWR, 0666);
    if (fd_shm == -1) {
        perror("Error en shm_open");
        return EXIT_FAILURE;
    }

    if (ftruncate(fd_shm, sizeof(struct MemoriaCompartida)) == -1) {
        perror("Error en ftruncate");
        limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
        return EXIT_FAILURE;
    }

    shm = (struct MemoriaCompartida *)mmap(NULL, sizeof(struct MemoriaCompartida),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (shm == MAP_FAILED) {
        perror("Error en mmap");
        limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
        return EXIT_FAILURE;
    }

    memset(shm, 0, sizeof(struct MemoriaCompartida));
    shm->total_sensores = num_sensores;
    shm->total_esperado = num_sensores * mediciones_por_sensor;

    /* 3. Inicialización de Semáforo POSIX con Nombre (Mutex Interproceso) */
    sem_mutex = sem_open(NOMBRE_SEM, O_CREAT | O_EXCL, 0666, 1);
    if (sem_mutex == SEM_FAILED) {
        perror("Error en sem_open");
        limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
        return EXIT_FAILURE;
    }

    /* 4. Inicialización de Cola de Mensajes POSIX */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MENSAJES_COLA;
    attr.mq_msgsize = sizeof(struct Medicion);
    attr.mq_curmsgs = 0;

    mq = mq_open(NOMBRE_COLA, O_CREAT | O_EXCL | O_RDWR, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("Error en mq_open");
        limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
        return EXIT_FAILURE;
    }

    /* 5. Creación del Pool de Worker Threads */
    for (i = 0; i < num_workers; i++) {
        args[i].id = i + 1;
        args[i].mq = mq;
        args[i].shm = shm;
        args[i].sem_mutex = sem_mutex;

        if (pthread_create(&workers[i], NULL, worker_function, &args[i]) != 0) {
            perror("Error al crear worker thread");
            limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
            return EXIT_FAILURE;
        }
    }

    /* 6. Creación de Procesos Sensores mediante fork() */
    for (i = 0; i < num_sensores; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Error en fork");
            limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
            return EXIT_FAILURE;
        } else if (pids[i] == 0) {
            /* Proceso Hijo (Sensor) */
            ejecutar_sensor(i + 1, mediciones_por_sensor);
        }
    }

    /* 7. Espera de finalización de procesos sensores hijos */
    for (i = 0; i < num_sensores; i++) {
        waitpid(pids[i], NULL, 0);
    }
    printf("[Colector] Todos los procesos sensores han finalizado el envío.\n");

    /* 8. Espera de finalización del Pool de Worker Threads */
    for (i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }
    printf("[Colector] Todos los worker threads han procesado sus mediciones.\n");

    /* 9. Resumen Final de Estadísticas en Consola */
    printf("\n=====================================================\n");
    printf(" RESUMEN FINAL DE ESTADÍSTICAS EN MEMORIA COMPARTIDA\n");
    printf("=====================================================\n");
    printf("Mediciones procesadas totales: %d / %d\n\n",
           shm->total_mediciones_recibidas, shm->total_esperado);

    for (i = 0; i < shm->total_sensores; i++) {
        if (shm->sensores[i].cantidad > 0) {
            printf("Sensor ID: %02d | Muestras: %d | Prom: %.2f | Min: %.2f | Max: %.2f | Último: %.2f\n",
                   shm->sensores[i].sensor_id,
                   shm->sensores[i].cantidad,
                   shm->sensores[i].suma / shm->sensores[i].cantidad,
                   shm->sensores[i].min,
                   shm->sensores[i].max,
                   shm->sensores[i].ultimo_valor);
        }
    }
    printf("=====================================================\n");

    /* 10. Liberación Ordenada de Recursos IPC */
    limpiar_recursos_ipc(mq, sem_mutex, shm, fd_shm);
    printf("[Colector] Recursos IPC liberados correctamente. Fin de ejecución.\n");

    return EXIT_SUCCESS;
}
