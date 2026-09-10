/*
 * comun.h - Definiciones comunes para el sistema SCADA (Parte I)
 *
 * Asignatura: Laboratorio III
 * Estándar: C89 / ANSI C (-std=c89 -pedantic -Wall -Wextra -Werror)
 */

#ifndef COMUN_H
#define COMUN_H

/* Macros de compatibilidad POSIX para C89 */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <stddef.h>

/* Nombres para recursos IPC POSIX (deben iniciar con '/') */
#define NOMBRE_COLA "/mq_scada_mediciones"
#define NOMBRE_SHM  "/shm_scada_stats"
#define NOMBRE_SEM  "/sem_scada_mutex"

/* Límites del sistema */
#define MAX_SENSORES 32
#define MAX_MENSAJES_COLA 10

/* Estructura de medición enviada por los sensores vía Cola de Mensajes */
struct Medicion {
    int sensor_id;
    double valor;
    long timestamp;
};

/* Estadísticas individuales almacenadas por cada sensor */
struct StatsSensor {
    int sensor_id;
    int cantidad;
    double suma;
    double min;
    double max;
    double ultimo_valor;
};

/* Estructura principal almacenada en Memoria Compartida */
struct MemoriaCompartida {
    int total_sensores;
    int total_mediciones_recibidas;
    int total_esperado;
    struct StatsSensor sensores[MAX_SENSORES];
};

#endif /* COMUN_H */
