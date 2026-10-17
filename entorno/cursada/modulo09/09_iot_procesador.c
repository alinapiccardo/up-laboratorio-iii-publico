/*
 * 09_iot_procesador.c - Thread procesador (consumidor)
 *
 * Recibe mediciones de la cola con mq_receive (bloqueante) hasta que
 * llegaron tantos SENSOR_FIN como sensores hay.
 */
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "09_iot.h"

/* Formatea un time_t como HH:MM:SS (static: solo se usa aca) */
static void formatear_hora(time_t t, char *buf, size_t tam) {
  struct tm tm_local;

  localtime_r(&t, &tm_local);
  strftime(buf, tam, "%H:%M:%S", &tm_local);
}

void *funcion_procesador(void *arg) {
  struct DatosProcesador *datos = (struct DatosProcesador *)arg;
  struct Medicion med;
  int sensores_finalizados = 0;
  int procesadas = 0;
  char hora[16];

  printf("[Procesador] Iniciado, esperando mediciones de %d sensores\n",
         datos->num_sensores);

  while (sensores_finalizados < datos->num_sensores) {
    /* Bloqueante: si la cola esta vacia, espera. No hace falta mutex. */
    if (mq_receive(datos->cola, (char *)&med, sizeof(med), NULL) == -1) {
      perror("mq_receive");
      pthread_exit(NULL);
    }

    if (med.sensor_id == SENSOR_FIN) {
      sensores_finalizados++;
      printf("[Procesador] Sensor finalizado (%d/%d)\n", sensores_finalizados,
             datos->num_sensores);
      continue;
    }

    procesadas++;
    formatear_hora(med.timestamp, hora, sizeof(hora));
    printf("[Procesador] #%03d | Sensor %d | %s | %.1f %s | %s\n", procesadas,
           med.sensor_id, nombre_sensor(med.tipo), med.valor,
           unidad_sensor(med.tipo), hora);
  }

  printf("[Procesador] Finalizado. Total de mediciones procesadas: %d\n",
         procesadas);
  return NULL;
}