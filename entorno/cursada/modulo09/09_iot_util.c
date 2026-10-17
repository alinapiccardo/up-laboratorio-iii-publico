/*
 * 09_iot_util.c - Funciones auxiliares que usan el sensor y el procesador
 */
#include "09_iot.h"

const char *nombre_sensor(int tipo) {
  switch (tipo) {
  case SENSOR_TEMPERATURA:
    return "Temperatura";
  case SENSOR_HUMEDAD:
    return "Humedad";
  case SENSOR_PRESION:
    return "Presion";
  default:
    return "Desconocido";
  }
}

const char *unidad_sensor(int tipo) {
  switch (tipo) {
  case SENSOR_TEMPERATURA:
    return "C";
  case SENSOR_HUMEDAD:
    return "%";
  case SENSOR_PRESION:
    return "hPa";
  default:
    return "?";
  }
}