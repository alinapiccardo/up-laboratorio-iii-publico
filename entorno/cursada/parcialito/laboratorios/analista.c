/* analista: consumidor. Abre los recursos creados por recepcion (debe
 * ejecutarse recepcion primero). Ejecutar: ./analista */
#include "laboratorio.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
  int fd, t, fin;
  Laboratorio *lab;
  sem_t *v, *l, *m;

  fd = shm_open(SHM, O_RDWR, 0600);
  if (fd == -1) {
    perror("shm (ejecute ./recepcion primero)");
    exit(1);
  }
  lab = mmap(NULL, sizeof(Laboratorio), PROT_READ | PROT_WRITE, MAP_SHARED, fd,
             0);
  close(fd);

  v = sem_open(SVACIOS, 0);
  l = sem_open(SLLENOS, 0);
  m = sem_open(SMUTEX, 0);

  fin = 0;
  while (!fin) {
    sem_wait(l); /* espera si el area de admision esta vacia */
    sem_wait(m);
    t = lab->tipo[lab->out];
    lab->out = (lab->out + 1) % CAP;
    if (t == FIN) {
      fin = 1;
    } else {
      lab->total = lab->total + 1;
      if (t) {
        lab->urgentes = lab->urgentes + 1;
      } else {
        lab->rutinarias = lab->rutinarias + 1;
      }
    }
    sem_post(m);
    sem_post(v);
    if (!fin) {
      printf("analista: tipo=%d total=%lu rutinarias=%lu "
             "urgentes=%lu\n",
             t, lab->total, lab->rutinarias, lab->urgentes);
      usleep(150000);
    }
  }

  printf("analista: FIN. total=%lu rutinarias=%lu urgentes=%lu\n", lab->total,
         lab->rutinarias, lab->urgentes);

  munmap(lab, sizeof(Laboratorio));
  shm_unlink(SHM);
  sem_unlink(SVACIOS);
  sem_unlink(SLLENOS);
  sem_unlink(SMUTEX);
  return 0;
}