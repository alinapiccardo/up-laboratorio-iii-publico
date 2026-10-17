/*
 * compartido.h
 *
 * Header compartido entre cliente (productor) e impresora (consumidor).
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
#define SHM_NOMBRE          "/spooler_impresion_buffer"
#define SEM_VACIOS_NOMBRE   "/spooler_impresion_vacios"
#define SEM_LLENOS_NOMBRE   "/spooler_impresion_llenos"
#define SEM_MUTEX_NOMBRE    "/spooler_impresion_mutex"

/*
 * Capacidad del buffer circular.
 * Define cuantas paginas caben antes de que el cliente se bloquee.
 */
#define BUFFER_CAPACIDAD 5

/* Tipos de pagina */
#define PAGINA_BN    0
#define PAGINA_COLOR 1

/*
 * Estructura que representa una pagina a imprimir.
 */
struct Pagina {
    int id;   /* identificador secuencial de la pagina */
    int tipo; /* PAGINA_BN o PAGINA_COLOR */
};

/*
 * Estructura que representa el buffer compartido completo.
 * Esta estructura es lo que vive en la memoria compartida.
 */
struct BufferCompartido {
    struct Pagina paginas[BUFFER_CAPACIDAD];
    int cabeza;              /* indice donde el cliente escribe */
    int cola;                 /* indice donde la impresora lee */
    unsigned long total;     /* total de paginas impresas (lo actualiza la impresora) */
    unsigned long bn;        /* cuantas fueron blanco y negro */
    unsigned long color;     /* cuantas fueron color */
};

#endif /* COMPARTIDO_H */
