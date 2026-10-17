#ifndef SHARED_H
#define SHARED_H

#define SHM_NAME "/shm_spooler"
#define SEM_VACIOS "/sem_vacios"
#define SEM_LLENOS "/sem_llenos"
#define SEM_MUTEX "/sem_mutex"

#define CAPACIDAD 5
#define FIN_IMPRESION (-1)

typedef struct {
    int buffer[CAPACIDAD];
    int in;
    int out;
    int total_impresas;
    int total_byn;
    int total_color;
} spooler_t;

#endif
