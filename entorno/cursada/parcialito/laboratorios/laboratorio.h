#ifndef _LABORATORIO_H
#define _LABORATORIO_H

#define CAP 5
#define SHM "/lab_shm"
#define SVACIOS "/lab_vacios"
#define SLLENOS "/lab_llenos"
#define SMUTEX "/lab_mutex"
#define FIN (-1) /* tipo especial: avisa que no hay mas muestras */

typedef struct {
  int tipo[CAP]; /* 0 = rutinaria, 1 = urgente */
  int in, out;
  unsigned long total, rutinarias, urgentes;
} Laboratorio;

#endif