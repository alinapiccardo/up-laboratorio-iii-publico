#ifndef COMPARTIDO_H
#define COMPARTIDO_H

/*
 * Nombres de los recursos IPC.
 * Todos deben empezar con '/' segun POSIX.
 */
#define SHM_NOMBRE "/laboratorio_muestras_buffer"
#define SEM_VACIOS_NOMBRE "/laboratorio_muestras_vacios"
#define SEM_LLENOS_NOMBRE "/laboratorio_muestras_llenos"
#define SEM_MUTEX_NOMBRE "/laboratorio_muestras_mutex"

/*
 * Capacidad del buffer circular.
 * Define cuantas muestras caben antes de que recepcion se bloquee.
 */
#define BUFFER_CAPACIDAD 5

/* Tipos de muestra */
#define MUESTRA_RUTINARIA 0
#define MUESTRA_URGENTE 1

/*
 * Estructura que representa una muestra de paciente.
 */
struct Muestra {
  int id;   /* identificador secuencial de la muestra */
  int tipo; /* MUESTRA_RUTINARIA o MUESTRA_URGENTE */
};

/*
 * Estructura que representa el buffer compartido completo.
 * Esta estructura es lo que vive en la memoria compartida.
 */
struct BufferCompartido {
  struct Muestra muestras[BUFFER_CAPACIDAD];
  int cabeza; /* indice donde recepcion escribe */
  int cola;   /* indice donde el analista lee */
  unsigned long
      total; /* total de muestras procesadas (lo actualiza el analista) */
  unsigned long rutinarias; /* cuantas fueron rutinarias */
  unsigned long urgentes;   /* cuantas fueron urgentes */
};

#endif /* COMPARTIDO_H */
