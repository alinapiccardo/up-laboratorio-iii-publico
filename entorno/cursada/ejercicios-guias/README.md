# Ejercitación Práctica: Sistema de Pedidos de Restaurante (`pedidos.c`)

Materia: Laboratorio III
Alumna: Alina Piccardo

Implementa un sistema en C que simula la recepción y preparación de pedidos de un restaurante de delivery, utilizando persistencia en un archivo de texto (`pedidos.txt`).

---

## 📌 Descripción del Programa

El programa simula el flujo de trabajo real de un restaurante dividido en dos fases independientes:

1. **Recepción de pedidos**: Los clientes realizan pedidos que se registran secuencialmente en el archivo `pedidos.txt`.
2. **Preparación de pedidos**: La cocina toma los pedidos guardados en `pedidos.txt`, los lee línea por línea y simula su preparación procesándolos en pantalla.

---

## ⚙️ Restricciones Técnicas

- **Punteros `FILE` independientes**:
  - `f_escritura`: Utilizado exclusivamente en la función `recibir_pedidos`.
  - `f_lectura`: Utilizado exclusivamente en la función `preparar_pedidos`.
- **Modos de apertura**:
  - Modo `"w"` (write) para la fase de recepción/escritura (crea o sobrescribe el archivo).
  - Modo `"r"` (read) para la fase de preparación/lectura (lectura de líneas con `fgets`).
- **Funciones separadas**:
  - `recibir_pedidos(...)`: Registra los 10 pedidos en el archivo.
  - `preparar_pedidos(...)`: Procesa y muestra los pedidos desde el archivo.
- **Validación de errores y recursos**:
  - Verificación estricta del retorno de `fopen` (`!= NULL`) antes de cualquier operación.
  - Cierre explícito de archivos con `fclose` al finalizar cada fase.
- **Estándar C89**:
  - Compilado con flags estrictos sin advertencias ni errores: `gcc -std=c89 -pedantic -Wall -Werror -o pedidos pedidos.c`.

---

## 📄 Formato del Archivo (`pedidos.txt`)

Cada pedido ocupa exactamente una línea con el formato numerado secuencialmente a tres dígitos (`PEDIDO-XXX: <Descripción>`):

```text
PEDIDO-001: Hamburguesa con papas
PEDIDO-002: Pizza grande de muzzarella
PEDIDO-003: Ensalada Caesar
PEDIDO-004: Empanadas de carne (docena)
PEDIDO-005: Milanesa a la napolitana con fritas
PEDIDO-006: Combo de sushi (15 piezas)
PEDIDO-007: Tacos de carne asada
PEDIDO-008: Lomito completo
PEDIDO-009: Spaghetti a la bolognesa
PEDIDO-010: Flan casero con dulce de leche
```

---

## 🛠️ Instrucciones de Compilación y Ejecución

### Opción 1: Comando de Compilación Directo (según consigna)
```bash
gcc -std=c89 -pedantic -Wall -Werror -o pedidos pedidos.c
./pedidos
```

### Opción 2: Uso del Makefile
Para compilar:
```bash
make
```

Para compilar y ejecutar directamente:
```bash
make run
```

Para limpiar ejecutables y archivos temporales (`pedidos.txt`):
```bash
make clean
```

---

## 🖥️ Ejemplo de Salida del Programa

```text
-----------------------------------------------------
   SISTEMA DE GESTIÓN DE PEDIDOS DE RESTAURANTE      
-----------------------------------------------------

=====================================================
1. FASE DE RECEPCIÓN DE PEDIDOS (Escritura en 'w')
=====================================================
[RECEPCIÓN] Registrado -> PEDIDO-001: Hamburguesa con papas
[RECEPCIÓN] Registrado -> PEDIDO-002: Pizza grande de muzzarella
[RECEPCIÓN] Registrado -> PEDIDO-003: Ensalada Caesar
[RECEPCIÓN] Registrado -> PEDIDO-004: Empanadas de carne (docena)
[RECEPCIÓN] Registrado -> PEDIDO-005: Milanesa a la napolitana con fritas
[RECEPCIÓN] Registrado -> PEDIDO-006: Combo de sushi (15 piezas)
[RECEPCIÓN] Registrado -> PEDIDO-007: Tacos de carne asada
[RECEPCIÓN] Registrado -> PEDIDO-008: Lomito completo
[RECEPCIÓN] Registrado -> PEDIDO-009: Spaghetti a la bolognesa
[RECEPCIÓN] Registrado -> PEDIDO-010: Flan casero con dulce de leche
--> Éxito: Se registraron 10 pedidos en 'pedidos.txt'.

=====================================================
2. FASE DE PREPARACIÓN DE PEDIDOS EN COCINA (Lectura 'r')
=====================================================
[COCINA] Preparando y listo -> PEDIDO-001: Hamburguesa con papas
[COCINA] Preparando y listo -> PEDIDO-002: Pizza grande de muzzarella
[COCINA] Preparando y listo -> PEDIDO-003: Ensalada Caesar
[COCINA] Preparando y listo -> PEDIDO-004: Empanadas de carne (docena)
[COCINA] Preparando y listo -> PEDIDO-005: Milanesa a la napolitana con fritas
[COCINA] Preparando y listo -> PEDIDO-006: Combo de sushi (15 piezas)
[COCINA] Preparando y listo -> PEDIDO-007: Tacos de carne asada
[COCINA] Preparando y listo -> PEDIDO-008: Lomito completo
[COCINA] Preparando y listo -> PEDIDO-009: Spaghetti a la bolognesa
[COCINA] Preparando y listo -> PEDIDO-010: Flan casero con dulce de leche
--> Éxito: Se procesaron y prepararon 10 pedidos desde 'pedidos.txt'.

Proceso completado exitosamente.
```
