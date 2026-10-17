/* bodega: consumidor. Abre los recursos creados por checkin (debe
 * ejecutarse checkin primero). Ejecutar: ./bodega */
#include "cinta.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
  int fd, t, fin, peso;
  Cinta *c;
  sem_t *v, *l, *m;

  fd = shm_open(SHM, O_RDWR, 0600);
  if (fd == -1) {
    perror("shm (ejecute ./checkin primero)");
    exit(1);
  }
  c = mmap(NULL, sizeof(Cinta), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);

  v = sem_open(SVACIOS, 0);
  l = sem_open(SLLENOS, 0);
  m = sem_open(SMUTEX, 0);

  fin = 0;
  while (!fin) {
    sem_wait(l); /* espera si la cinta esta vacia */
    sem_wait(m);
    t = c->tipo[c->out];
    c->out = (c->out + 1) % CAP;
    if (t == FIN) {
      fin = 1;
    } else {
      peso = t ? 5 : 1;
      c->total = c->total + 1;
      c->peso = c->peso + (unsigned long)peso;
    }
    sem_post(m);
    sem_post(v);
    if (!fin) {
      printf("bodega: retira tipo=%d total=%lu peso=%lu\n", t, c->total,
             c->peso);
      usleep(150000);
    }
  }

  printf("bodega: FIN. total=%lu peso=%lu\n", c->total, c->peso);

  munmap(c, sizeof(Cinta));
  shm_unlink(SHM);
  sem_unlink(SVACIOS);
  sem_unlink(SLLENOS);
  sem_unlink(SMUTEX);
  return 0;
}