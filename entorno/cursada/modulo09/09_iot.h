/*
 * 09_iot.h - Definiciones compartidas del sistema IoT
 * (threads sensores -> cola POSIX -> thread procesador)
 *
 * Lo incluyen todos los modulos:
 *   09_iot_util.c       funciones auxiliares comunes
 *   09_iot_sensor.c     thread sensor (productor)
 *   09_iot_procesador.c thread procesador (consumidor)
 *   09_iot_main.c       creacion de la cola y de los threads
 */
#ifndef IOT_H
#define IOT_H

#include <mqueue.h>
#include <time.h>

/* Parametros de la cola */
#define COLA_SENSORES "/sensores_iot"
#define COLA_MAX_MENSAJES 10
#define COLA_TAM_MENSAJE sizeof(struct Medicion)

/* Parametros del sistema */
#define MAX_SENSORES 8
#define MEDICIONES_POR_SENSOR 5

/* Tipos de sensor */
#define SENSOR_TEMPERATURA 0
#define SENSOR_HUMEDAD 1
#define SENSOR_PRESION 2

/* Valor especial de sensor_id: "este sensor termino" */
#define SENSOR_FIN (-1)

/* Mensaje que viaja por la cola */
struct Medicion {
  int sensor_id;    /* quien envia (o SENSOR_FIN)  */
  int tipo;         /* SENSOR_TEMPERATURA, etc.    */
  double valor;     /* valor medido                */
  time_t timestamp; /* momento de la medicion      */
};

/* Argumento de cada thread sensor */
struct DatosSensor {
  int id;
  int tipo;
  mqd_t cola; /* descriptor de la cola compartida */
  int num_mediciones;
  unsigned int semilla; /* semilla propia para rand_r()     */
};

/* Argumento del thread procesador */
struct DatosProcesador {
  mqd_t cola;
  int num_sensores; /* cuantos SENSOR_FIN esperar */
};

/* --- 09_iot_util.c ------------------------------------------------- */
const char *nombre_sensor(int tipo);
const char *unidad_sensor(int tipo);

/* --- 09_iot_sensor.c ----------------------------------------------- */
void *funcion_sensor(void *arg);

/* --- 09_iot_procesador.c ------------------------------------------- */
void *funcion_procesador(void *arg);

#endif /* IOT_H */