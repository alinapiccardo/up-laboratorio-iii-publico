/*
 * 09_iot_main.c - Creacion y coordinacion del sistema IoT
 *
 * Crea la cola POSIX, lanza el thread procesador y despues los
 * threads sensores, espera a que terminen y libera los recursos.
 *
 * Uso: ./09_iot_sistema [num_sensores]     (1..MAX_SENSORES, default 3)
 */
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "09_iot.h"

int main(int argc, char *argv[]) {
  int num_sensores = 3;
  int creados = 0;
  mqd_t cola;
  struct mq_attr attr;
  pthread_t *hilos_sensores;
  pthread_t hilo_procesador;
  struct DatosSensor *datos_sensores;
  struct DatosProcesador datos_procesador;
  struct Medicion fin;
  char *resto;
  long n;
  int i, rc;

  /* Argumento opcional: cantidad de sensores */
  if (argc > 1) {
    n = strtol(argv[1], &resto, 10);
    if (*resto != '\0' || n < 1 || n > MAX_SENSORES) {
      fprintf(stderr, "Uso: %s [num_sensores 1..%d]\n", argv[0], MAX_SENSORES);
      return EXIT_FAILURE;
    }
    num_sensores = (int)n;
  }

  printf("==============================================\n");
  printf("SISTEMA IoT - %d sensores\n", num_sensores);
  printf("Threads + Colas de Mensajes POSIX\n");
  printf("==============================================\n\n");

  /* Eliminar una cola que haya quedado de una corrida anterior */
  mq_unlink(COLA_SENSORES);

  attr.mq_flags = 0;
  attr.mq_maxmsg = COLA_MAX_MENSAJES;
  attr.mq_msgsize = COLA_TAM_MENSAJE;
  attr.mq_curmsgs = 0;

  cola = mq_open(COLA_SENSORES, O_CREAT | O_RDWR, 0600, &attr);
  if (cola == (mqd_t)-1) {
    perror("mq_open");
    return EXIT_FAILURE;
  }
  printf("[Main] Cola de mensajes creada: %s\n", COLA_SENSORES);

  hilos_sensores = malloc(num_sensores * sizeof(pthread_t));
  datos_sensores = malloc(num_sensores * sizeof(struct DatosSensor));
  if (hilos_sensores == NULL || datos_sensores == NULL) {
    perror("malloc");
    free(hilos_sensores);
    free(datos_sensores);
    mq_close(cola);
    mq_unlink(COLA_SENSORES);
    return EXIT_FAILURE;
  }

  /* El consumidor se crea ANTES que los productores */
  datos_procesador.cola = cola;
  datos_procesador.num_sensores = num_sensores;
  rc = pthread_create(&hilo_procesador, NULL, funcion_procesador,
                      &datos_procesador);
  if (rc != 0) {
    fprintf(stderr, "pthread_create (procesador): error %d\n", rc);
    free(hilos_sensores);
    free(datos_sensores);
    mq_close(cola);
    mq_unlink(COLA_SENSORES);
    return EXIT_FAILURE;
  }

  printf("[Main] Creando %d threads sensores...\n\n", num_sensores);
  for (i = 0; i < num_sensores; i++) {
    datos_sensores[i].id = i;
    datos_sensores[i].tipo = i % 3;
    datos_sensores[i].cola = cola;
    datos_sensores[i].num_mediciones = MEDICIONES_POR_SENSOR;
    datos_sensores[i].semilla =
        (unsigned int)time(NULL) ^ ((unsigned int)i * 7919u);

    rc = pthread_create(&hilos_sensores[i], NULL, funcion_sensor,
                        &datos_sensores[i]);
    if (rc != 0) {
      fprintf(stderr, "pthread_create (sensor %d): error %d\n", i, rc);
      break;
    }
    creados++;
  }

  /*
   * Si algun sensor no se pudo crear, el procesador esperaria para
   * siempre su SENSOR_FIN: lo mandamos desde main en su lugar.
   */
  for (i = creados; i < num_sensores; i++) {
    fin.sensor_id = SENSOR_FIN;
    fin.tipo = 0;
    fin.valor = 0.0;
    fin.timestamp = time(NULL);
    mq_send(cola, (const char *)&fin, sizeof(fin), 0);
  }

  /* Esperar a todos */
  for (i = 0; i < creados; i++) {
    pthread_join(hilos_sensores[i], NULL);
  }
  pthread_join(hilo_procesador, NULL);

  printf("\n[Main] Todos los threads finalizaron\n");

  /* Limpiar */
  free(hilos_sensores);
  free(datos_sensores);
  mq_close(cola);
  mq_unlink(COLA_SENSORES);

  printf("[Main] Recursos liberados\n");
  printf("==============================================\n");

  return (creados == num_sensores) ? EXIT_SUCCESS : EXIT_FAILURE;
}