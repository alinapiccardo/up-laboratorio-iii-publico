/* check-in: productor. Crea la memoria compartida y los semaforos. */
#include "cinta.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define N 20 /* cantidad de valijas a cargar */

int main(void) {
  int fd, i, t;
  Cinta *c;
  sem_t *v, *l, *m;

  /* Limpiar recursos de ejecuciones anteriores si quedaron colgados */
  shm_unlink(SHM);
  sem_unlink(SVACIOS);
  sem_unlink(SLLENOS);
  sem_unlink(SMUTEX);

  fd = shm_open(SHM, O_CREAT | O_RDWR, 0600);
  if (fd == -1 || ftruncate(fd, sizeof(Cinta)) == -1) {
    perror("shm");
    exit(1);
  }
  c = mmap(NULL, sizeof(Cinta), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  memset(c, 0, sizeof(Cinta));

  v = sem_open(SVACIOS, O_CREAT, 0600, CAP);
  l = sem_open(SLLENOS, O_CREAT, 0600, 0);
  m = sem_open(SMUTEX, O_CREAT, 0600, 1);

  srand((unsigned)time(NULL));

  for (i = 0; i < N; i++) {
    t = rand() % 2; /* 0 = cabina, 1 = bodega */

    sem_wait(v); /* espera si la cinta esta llena */
    sem_wait(m);
    c->tipo[c->in] = t;
    c->in = (c->in + 1) % CAP;
    sem_post(m);
    sem_post(l);

    printf("check-in: valija %d tipo=%d\n", i, t);
    usleep(100000);
  }

  /* aviso de fin, mismo protocolo que una valija normal */
  sem_wait(v);
  sem_wait(m);
  c->tipo[c->in] = FIN;
  c->in = (c->in + 1) % CAP;
  sem_post(m);
  sem_post(l);

  munmap(c, sizeof(Cinta));
  return 0;
}