/* recepcion: productor. Crea la memoria compartida y los semaforos.
 *
 * IMPORTANTE: antes de crear, borra cualquier recurso viejo que haya
 * quedado de una corrida anterior interrumpida (por ejemplo con Ctrl+C).
 * Si no se hace esto, un semaforo puede quedar con un valor invalido
 * (por ej. el mutex en 0 porque el proceso murio dentro de la seccion
 * critica) y la proxima corrida se queda bloqueada para siempre en
 * sem_wait, aunque el codigo este bien.
 *
 * Compilar: ver Makefile. Ejecutar: ./recepcion */
#include "laboratorio.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define N 20 /* cantidad de muestras a registrar */

int main(void) {
  int fd, i, t;
  Laboratorio *lab;
  sem_t *v, *l, *m;

  /* limpieza de una corrida anterior (se ignora si no existian) */
  shm_unlink(SHM);
  sem_unlink(SVACIOS);
  sem_unlink(SLLENOS);
  sem_unlink(SMUTEX);

  fd = shm_open(SHM, O_CREAT | O_RDWR, 0600);
  if (fd == -1 || ftruncate(fd, sizeof(Laboratorio)) == -1) {
    perror("shm");
    exit(1);
  }
  lab = mmap(NULL, sizeof(Laboratorio), PROT_READ | PROT_WRITE, MAP_SHARED, fd,
             0);
  close(fd);
  memset(lab, 0, sizeof(Laboratorio));

  v = sem_open(SVACIOS, O_CREAT, 0600, CAP);
  l = sem_open(SLLENOS, O_CREAT, 0600, 0);
  m = sem_open(SMUTEX, O_CREAT, 0600, 1);

  srand((unsigned)time(NULL));

  for (i = 0; i < N; i++) {
    t = rand() % 2; /* 0 = rutinaria, 1 = urgente */

    sem_wait(v); /* espera si el area de admision esta llena */
    sem_wait(m);
    lab->tipo[lab->in] = t;
    lab->in = (lab->in + 1) % CAP;
    sem_post(m);
    sem_post(l);

    printf("recepcion: muestra %d tipo=%d\n", i, t);
    usleep(100000);
  }

  /* aviso de fin, mismo protocolo que una muestra normal */
  sem_wait(v);
  sem_wait(m);
  lab->tipo[lab->in] = FIN;
  lab->in = (lab->in + 1) % CAP;
  sem_post(m);
  sem_post(l);

  munmap(lab, sizeof(Laboratorio));
  return 0;
}