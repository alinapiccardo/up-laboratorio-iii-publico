/*
 * 09_iot_sensor.c - Thread sensor (productor)
 *
 * Genera mediciones simuladas, las envia a la cola con mq_send y al
 * terminar manda un mensaje con sensor_id = SENSOR_FIN.
 */
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "09_iot.h"

/*
 * Genera un valor aleatorio dentro de un rango razonable para el tipo.
 * Usa rand_r() con una semilla por thread: rand() comparte estado
 * global entre threads y no es thread-safe.
 * Es static: solo se usa en este archivo.
 */
static double generar_valor_simulado(int tipo, unsigned int *semilla) {
  double r = (double)rand_r(semilla) / (double)RAND_MAX; /* 0.0 .. 1.0 */

  switch (tipo) {
  case SENSOR_TEMPERATURA:
    return 15.0 + r * 10.0; /* 15 a 25 C       */
  case SENSOR_HUMEDAD:
    return 40.0 + r * 20.0; /* 40 a 60 %       */
  case SENSOR_PRESION:
    return 1000.0 + r * 20.0; /* 1000 a 1020 hPa */
  default:
    return 0.0;
  }
}

/* Duerme 'ms' milisegundos (nanosleep es POSIX; usleep quedo obsoleto) */
static void dormir_ms(long ms) {
  struct timespec ts;

  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000L;
  while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
    /* interrumpido por una senal: dormir lo que falta */
  }
}

void *funcion_sensor(void *arg) {
  struct DatosSensor *datos = (struct DatosSensor *)arg;
  struct Medicion med;
  int i;

  printf("[Sensor %d] Iniciado (tipo: %s)\n", datos->id,
         nombre_sensor(datos->tipo));

  for (i = 0; i < datos->num_mediciones; i++) {
    med.sensor_id = datos->id;
    med.tipo = datos->tipo;
    med.valor = generar_valor_simulado(datos->tipo, &datos->semilla);
    med.timestamp = time(NULL);

    /* Si la cola esta llena, mq_send bloquea (backpressure) */
    if (mq_send(datos->cola, (const char *)&med, sizeof(med), 0) == -1) {
      perror("mq_send");
      pthread_exit(NULL);
    }
    printf("[Sensor %d] Enviado: %.1f %s\n", datos->id, med.valor,
           unidad_sensor(datos->tipo));

    dormir_ms(100 + (long)(rand_r(&datos->semilla) % 200)); /* 100-300 ms */
  }

  /* Mensaje de fin para que el procesador sepa que este sensor termino */
  med.sensor_id = SENSOR_FIN;
  med.tipo = datos->tipo;
  med.valor = 0.0;
  med.timestamp = time(NULL);
  if (mq_send(datos->cola, (const char *)&med, sizeof(med), 0) == -1) {
    perror("mq_send (FIN)");
    pthread_exit(NULL);
  }
  printf("[Sensor %d] Finalizado\n", datos->id);

  return NULL;
}