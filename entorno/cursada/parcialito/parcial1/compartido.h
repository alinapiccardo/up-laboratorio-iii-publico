/*
 * compartido.h
 *
 * Header compartido entre check-in (productor) y bodega (consumidor).
 * Define las constantes, el nombre de los recursos IPC, y las estructuras
 * de datos que viven en la memoria compartida.
 *
 * IMPORTANTE: ambos procesos deben incluir este mismo archivo para
 * garantizar que el layout de la memoria compartida sea identico.
 */

#ifndef COMPARTIDO_H
#define COMPARTIDO_H

/*
 * Nombres de los recursos IPC.
 * Todos deben empezar con '/' segun POSIX.
 */
#define SHM_NOMBRE "/cinta_equipaje_buffer"
#define SEM_VACIOS_NOMBRE "/cinta_equipaje_vacios"
#define SEM_LLENOS_NOMBRE "/cinta_equipaje_llenos"
#define SEM_MUTEX_NOMBRE "/cinta_equipaje_mutex"

/*
 * Capacidad del buffer circular.
 * Define cuantas valijas caben antes de que check-in se bloquee.
 */
#define BUFFER_CAPACIDAD 5

/* Tipos de valija */
#define TIPO_CABINA 0
#define TIPO_BODEGA 1

/* Peso segun el enunciado: cabina pesa 1, bodega pesa 5 */
#define PESO_CABINA 1
#define PESO_BODEGA 5

/*
 * Estructura que representa una valija.
 */
struct Valija {
  int id;   /* identificador secuencial de la valija */
  int tipo; /* TIPO_CABINA o TIPO_BODEGA */
};

/*
 * Estructura que representa el buffer compartido completo.
 * Esta estructura es lo que vive en la memoria compartida.
 */
struct BufferCompartido {
  struct Valija valijas[BUFFER_CAPACIDAD];
  int cabeza;          /* indice donde check-in escribe */
  int cola;            /* indice donde bodega lee */
  unsigned long total; /* total de valijas cargadas (lo actualiza bodega) */
  unsigned long peso;  /* peso total cargado (lo actualiza bodega) */
};

#endif /* COMPARTIDO_H */