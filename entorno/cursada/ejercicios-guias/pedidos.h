#ifndef _PEDIDOS_H
#define _PEDIDOS_H

#define NOMBRE_ARCHIVO_PEDIDOS "pedidos.txt"
#define TAM_DESCRIPCION 128
#define TAM_LINEA 256

#define SEM_PEDIDOS_MUTEX "/sem_pedidos_mutex"
#define SEM_PEDIDOS_ITEMS "/sem_pedidos_items"

void formatear_linea(int numero, const char *descripcion, char *destino);
int registrar_pedido(int numero, const char *descripcion);
int leer_proximo_pedido(int *numero, char *descripcion);
int contar_pedidos_existentes(void);
int inicializar_semaforos(void);
void cerrar_semaforos(void);
void destruir_semaforos(void);

#endif
