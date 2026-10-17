#include "shared.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
  int fd, pagina, fin;
  spooler_t *spooler;
  sem_t *vacios, *llenos, *mutex;
  spooler_t inicial = {{0}, 0, 0, 0, 0, 0};

  /* Limpieza previa para evitar objetos huerfanos de ejecuciones fallidas */
  shm_unlink(SHM_NAME);
  sem_unlink(SEM_VACIOS);
  sem_unlink(SEM_LLENOS);
  sem_unlink(SEM_MUTEX);

  /* 1. Crear memoria compartida */
  fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  /* 2. Definir tamano (Regla de oro de los apuntes) */
  if (ftruncate(fd, sizeof(spooler_t)) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  /* 3. Mapear en espacio de direcciones */
  spooler =
      mmap(NULL, sizeof(spooler_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (spooler == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  close(fd); /* Cerrar file descriptor (Regla de oro) */

  /* 4. Crear e inicializar semaforos */
  vacios = sem_open(SEM_VACIOS, O_CREAT, 0666, CAPACIDAD);
  llenos = sem_open(SEM_LLENOS, O_CREAT, 0666, 0);
  mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

  if (vacios == SEM_FAILED || llenos == SEM_FAILED || mutex == SEM_FAILED) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  /* Inicializar estado del spooler copiando el struct entero */
  *spooler = inicial;

  printf("Impresora lista. Esperando paginas...\n");

  fin = 0;
  while (!fin) {
    /* Regla critica de orden: primero sem_wait a los contadores, luego al mutex */
    sem_wait(llenos);
    sem_wait(mutex);

    /* Seccion critica: Retirar del buffer circular */
    pagina = (*spooler).buffer[(*spooler).out];
    (*spooler).out = ((*spooler).out + 1) % CAPACIDAD;

    if (pagina == FIN_IMPRESION) {
      fin = 1;
    } else {
      /* Actualizar acumuladores */
      (*spooler).total_impresas++;
      if (pagina == 0) {
        (*spooler).total_byn++;
      } else {
        (*spooler).total_color++;
      }
    }

    /* Salida de la seccion critica */
    sem_post(mutex);
    sem_post(vacios);

    if (!fin) {
      printf("Impreso: %s | Acumulado -> Total: %d (ByN: %d, Color: %d)\n",
             pagina == 0 ? "Blanco y Negro" : "Color", (*spooler).total_impresas,
             (*spooler).total_byn, (*spooler).total_color);
      sleep(2); /* Simula el tiempo de impresion */
    }
  }

  printf("Impresora finalizo. Total impreso: %d (ByN: %d, Color: %d)\n",
         (*spooler).total_impresas, (*spooler).total_byn, (*spooler).total_color);

  munmap(spooler, sizeof(spooler_t));
  shm_unlink(SHM_NAME);
  sem_unlink(SEM_VACIOS);
  sem_unlink(SEM_LLENOS);
  sem_unlink(SEM_MUTEX);
  return 0;
}
