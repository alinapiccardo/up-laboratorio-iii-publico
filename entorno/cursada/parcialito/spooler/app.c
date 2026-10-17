#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "shared.h"

int main(void) {
    int fd, i, total_paginas, pagina;
    spooler_t *spooler;
    sem_t *vacios, *llenos, *mutex;

    /* 1. Abrir memoria compartida existente */
    fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (Debe arrancar la impresora primero)");
        exit(EXIT_FAILURE);
    }

    /* 2. Mapear (Tamano ya fue seteado por la impresora) */
    spooler = mmap(NULL, sizeof(spooler_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (spooler == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);

    /* 3. Abrir semaforos existentes */
    vacios = sem_open(SEM_VACIOS, 0);
    llenos = sem_open(SEM_LLENOS, 0);
    mutex = sem_open(SEM_MUTEX, 0);

    if (vacios == SEM_FAILED || llenos == SEM_FAILED || mutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));
    printf("App iniciada. Enviando paginas al spooler...\n");

    total_paginas = 10;
    for (i = 0; i < total_paginas; i++) {
        pagina = rand() % 2; /* 0 = ByN, 1 = Color */

        /* Regla critica de orden: primero contador, despues mutex */
        sem_wait(vacios);
        sem_wait(mutex);

        /* Seccion critica: Depositar en el buffer circular */
        (*spooler).buffer[(*spooler).in] = pagina;
        (*spooler).in = ((*spooler).in + 1) % CAPACIDAD;
        printf("App encolo pagina (%d/%d): %s\n", i + 1, total_paginas, pagina == 0 ? "Blanco y Negro" : "Color");

        /* Salida de la seccion critica */
        sem_post(mutex);
        sem_post(llenos);

        sleep(1); /* Simula el tiempo que tarda la app en producir una nueva pagina */
    }

    /* Enviar aviso de fin de impresion */
    sem_wait(vacios);
    sem_wait(mutex);
    (*spooler).buffer[(*spooler).in] = FIN_IMPRESION;
    (*spooler).in = ((*spooler).in + 1) % CAPACIDAD;
    sem_post(mutex);
    sem_post(llenos);

    printf("App finalizo la cola de impresion.\n");
    munmap(spooler, sizeof(spooler_t));
    return 0;
}
