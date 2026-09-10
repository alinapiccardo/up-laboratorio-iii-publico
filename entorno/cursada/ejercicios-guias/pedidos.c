#define _POSIX_C_SOURCE 200809L
#include "pedidos.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static FILE *f_lectura = NULL;
static sem_t *sem_mutex = SEM_FAILED;
static sem_t *sem_items = SEM_FAILED;

int inicializar_semaforos(void) {
  int pedidos_existentes;

  if (sem_mutex != SEM_FAILED && sem_items != SEM_FAILED) {
    return 1;
  }

  pedidos_existentes = contar_pedidos_existentes();

  sem_mutex = sem_open(SEM_PEDIDOS_MUTEX, O_CREAT, 0666, 1);
  if (sem_mutex == SEM_FAILED) {
    perror("Error sem_open (sem_pedidos_mutex)");
    return 0;
  }

  sem_items = sem_open(SEM_PEDIDOS_ITEMS, O_CREAT, 0666,
                       (unsigned int)pedidos_existentes);
  if (sem_items == SEM_FAILED) {
    perror("Error sem_open (sem_pedidos_items)");
    sem_close(sem_mutex);
    sem_mutex = SEM_FAILED;
    return 0;
  }

  return 1;
}

void cerrar_semaforos(void) {
  if (sem_mutex != SEM_FAILED) {
    if (sem_close(sem_mutex) < 0) {
      perror("Error sem_close (sem_pedidos_mutex)");
    }
    sem_mutex = SEM_FAILED;
  }
  if (sem_items != SEM_FAILED) {
    if (sem_close(sem_items) < 0) {
      perror("Error sem_close (sem_pedidos_items)");
    }
    sem_items = SEM_FAILED;
  }
}

void destruir_semaforos(void) {
  cerrar_semaforos();
  sem_unlink(SEM_PEDIDOS_MUTEX);
  sem_unlink(SEM_PEDIDOS_ITEMS);
}

void formatear_linea(int numero, const char *descripcion, char *destino) {
  if (destino != NULL && descripcion != NULL) {
    sprintf(destino, "PEDIDO-%03d: %s", numero, descripcion);
  }
}

int registrar_pedido(int numero, const char *descripcion) {
  FILE *f_escritura;
  char linea[TAM_LINEA];

  if (descripcion == NULL) {
    return 0;
  }

  if (sem_mutex == SEM_FAILED || sem_items == SEM_FAILED) {
    if (!inicializar_semaforos()) {
      return 0;
    }
  }

  if (sem_wait(sem_mutex) < 0) {
    perror("Error sem_wait (sem_pedidos_mutex)");
    return 0;
  }

  f_escritura = fopen(NOMBRE_ARCHIVO_PEDIDOS, "a");
  if (f_escritura == NULL) {
    perror("Error al abrir pedidos.txt en modo append");
    sem_post(sem_mutex);
    return 0;
  }

  formatear_linea(numero, descripcion, linea);
  if (fprintf(f_escritura, "%s\n", linea) < 0) {
    perror("Error al escribir en pedidos.txt");
    fclose(f_escritura);
    sem_post(sem_mutex);
    return 0;
  }

  if (fclose(f_escritura) == EOF) {
    perror("Error al cerrar pedidos.txt despues de registrar");
    sem_post(sem_mutex);
    return 0;
  }

  if (sem_post(sem_mutex) < 0) {
    perror("Error sem_post (sem_pedidos_mutex)");
    return 0;
  }

  if (sem_post(sem_items) < 0) {
    perror("Error sem_post (sem_pedidos_items)");
    return 0;
  }

  return 1;
}

int leer_proximo_pedido(int *numero, char *descripcion) {
  char buffer[TAM_LINEA];
  int exito = 0;

  if (numero == NULL || descripcion == NULL) {
    return 0;
  }

  if (sem_mutex == SEM_FAILED || sem_items == SEM_FAILED) {
    if (!inicializar_semaforos()) {
      return 0;
    }
  }

  if (sem_wait(sem_items) < 0) {
    perror("Error sem_wait (sem_pedidos_items)");
    return 0;
  }

  if (sem_wait(sem_mutex) < 0) {
    perror("Error sem_wait (sem_pedidos_mutex)");
    sem_post(sem_items);
    return 0;
  }

  if (f_lectura == NULL) {
    f_lectura = fopen(NOMBRE_ARCHIVO_PEDIDOS, "r");
    if (f_lectura == NULL) {
      perror("Error al abrir pedidos.txt en modo lectura");
      sem_post(sem_mutex);
      sem_post(sem_items);
      return 0;
    }
  } else {
    clearerr(f_lectura);
  }

  while (fgets(buffer, sizeof(buffer), f_lectura) != NULL) {
    buffer[strcspn(buffer, "\r\n")] = '\0';
    if (strlen(buffer) == 0) {
      continue;
    }

    if (sscanf(buffer, "PEDIDO-%d: %[^\n]", numero, descripcion) == 2) {
      exito = 1;
      break;
    }
  }

  if (!exito) {
    sem_post(sem_items);
  }

  if (sem_post(sem_mutex) < 0) {
    perror("Error sem_post (sem_pedidos_mutex)");
    return 0;
  }

  return exito;
}

int contar_pedidos_existentes(void) {
  FILE *f;
  char buffer[TAM_LINEA];
  int cantidad = 0;
  int num_temp;
  char desc_temp[TAM_DESCRIPCION];

  f = fopen(NOMBRE_ARCHIVO_PEDIDOS, "r");
  if (f == NULL) {
    return 0;
  }

  while (fgets(buffer, sizeof(buffer), f) != NULL) {
    buffer[strcspn(buffer, "\r\n")] = '\0';
    if (strlen(buffer) > 0 &&
        sscanf(buffer, "PEDIDO-%d: %[^\n]", &num_temp, desc_temp) == 2) {
      cantidad++;
    }
  }

  fclose(f);
  return cantidad;
}
