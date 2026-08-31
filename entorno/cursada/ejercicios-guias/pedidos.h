#ifndef _PEDIDOS_H
#define _PEDIDOS_H

#define NOMBRE_ARCHIVO_PEDIDOS "pedidos.txt"
#define TAM_DESCRIPCION 128
#define TAM_LINEA 256

void formatear_linea(int numero, const char *descripcion, char *destino);
int registrar_pedido(int numero, const char *descripcion);

/*
 * Lee la siguiente linea del archivo 'pedidos.txt', extrae el numero y la
 * descripcion del pedido y los guarda en los punteros recibidos.
 * Devuelve 1 si pudo leer un pedido correctamente, 0 si llego al final del
 * archivo o si ocurrio un error.
 *
 * Mantiene la posicion de lectura entre llamadas utilizando una variable static
 * FILE *.
 */
int leer_proximo_pedido(int *numero, char *descripcion);
int contar_pedidos_existentes(void);

#endif
