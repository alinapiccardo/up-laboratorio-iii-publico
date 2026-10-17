#ifndef _CINTA_H
#define _CINTA_H

#define CAP 5
#define SHM "/cinta_shm"
#define SVACIOS "/cinta_vacios"
#define SLLENOS "/cinta_llenos"
#define SMUTEX "/cinta_mutex"
#define FIN (-1) /* tipo especial: avisa que no hay mas valijas */

typedef struct {
  int tipo[CAP]; /* 0 = cabina, 1 = bodega */
  int in, out;
  unsigned long total, peso;
} Cinta;

#endif