# Ejercitación Práctica: Sistema de Pedidos de Restaurante (Concurrencia e IPC POSIX)

**Materia:** Laboratorio III  
**Alumna:** Alina Piccardo  

---

## 1. 📌 Diseño General del Sistema

El sistema simula el flujo de trabajo concurrente entre dos procesos independientes que representan la recepción y la cocina de un restaurante de delivery.

- **Productor (`recepcion`):** Genera pedidos aleatorios y los registra de forma persistente en la sección crítica del buffer.
- **Consumidor (`cocina`):** Lee y procesa los pedidos del buffer uno a uno de forma secuencial.
- **Buffer Compartido (`pedidos.txt`):** Archivo de texto plano en el sistema de archivos que actúa como buffer de comunicación interproceso (IPC) persistente y no acotado (unbounded buffer).
- **Semáforos POSIX Nombrados:** Dos semáforos globales identificados en `/dev/shm/`:
  - `SEM_PEDIDOS_MUTEX` (`"/sem_pedidos_mutex"`): Semáforo binario para exclusión mutua.
  - `SEM_PEDIDOS_ITEMS` (`"/sem_pedidos_items"`): Semáforo contador para la sincronización de recursos.

---

## 2. ⚙️ Decisiones Tomadas y Justificación

### a. Cantidad y Propósito de los Semáforos
Se implementaron **dos semáforos POSIX nombrados** mediante `sem_open`:
1. `sem_mutex` (`SEM_PEDIDOS_MUTEX`): **Semáforo binario**. Proporciona **exclusión mutua** sobre el archivo `pedidos.txt`. Garantiza que solo un proceso (sea recepción o cocina) acceda al archivo a la vez para evitar colisiones I/O y corrupción de datos.
2. `sem_items` (`SEM_PEDIDOS_ITEMS`): **Semáforo contador**. Modela la cantidad de pedidos disponibles en el buffer pendientes de ser preparados. Administra la sincronización según el patrón Productor-Consumidor.

### b. Valores Iniciales
- `sem_mutex`: Inicializado en **`1`**. Permite que el primer proceso que intente acceder al archivo pueda ingresar a la **sección crítica** inmediatamente.
- `sem_items`: Inicializado en la cantidad de pedidos no procesados en `pedidos.txt` (mediante `contar_pedidos_existentes()`). Si el sistema inicia con un archivo `pedidos.txt` vacío, su valor inicial es **`0`**. De este modo, si la cocina arranca con pedidos previos, no los ignora, y si no hay pedidos, se bloquea de inmediato.

### c. Ubicación de `sem_wait` y `sem_post`
- **Productor (`recepcion` / `registrar_pedido`)**:
  - `sem_wait(sem_mutex)`: Se ejecuta antes de realizar `fopen` en modo append (`"a"`).
  - Escrita de la línea formateada y `fclose`.
  - `sem_post(sem_mutex)`: Se ejecuta inmediatamente después de cerrar el archivo para liberar la sección crítica.
  - `sem_post(sem_items)`: Se ejecuta al finalizar para señalar la disponibilidad de un nuevo pedido e incrementar el contador de ítems consumibles.
- **Consumidor (`cocina` / `leer_proximo_pedido`)**:
  - `sem_wait(sem_items)`: Se ejecuta en primer lugar. Si el contador es 0, suspende al proceso en **espera bloqueante** (sin consumir CPU ni realizar *busy waiting* o *polling*).
  - `sem_wait(sem_mutex)`: Adquiere el cerrojo de exclusión mutua para ingresar a la sección crítica.
  - Lectura de la línea con `fgets` (aplicando `clearerr()` al stream estático para limpiar la bandera `EOF` cuando se agregan nuevos pedidos).
  - `sem_post(sem_mutex)`: Libera la sección crítica.

### d. Capacidad del Buffer
La capacidad del buffer está definida por el límite del sistema de archivos sobre el archivo `pedidos.txt`, actuando como un **buffer no acotado** (*unbounded buffer*). Por esta razón, no se requiere un semáforo contador de espacios vacíos (`sem_espacios`), ya que el productor nunca se bloquea por buffer lleno.

### e. Cierre Ordenado y Limpieza
- **Limpieza en Ejecución Normal**: Al finalizar la ejecución, ambos procesos invocan `cerrar_semaforos()` (`sem_close`), liberando los descriptores de semáforo del proceso.
- **Manejo de Señales**: Se instalaron manejadores para `SIGINT` (Ctrl+C) y `SIGTERM` mediante `signal()`. Ante una interrupción manual, se ejecuta `destruir_semaforos()`, la cual invoca `sem_unlink()` para remover los semáforos de la memoria compartida (`/dev/shm/sem.*`), evitando dejar semáforos huérfanos en el sistema.
- **Target Clean del Makefile**: La regla `make clean` incluye la eliminación explícita de `pedidos.txt` y `/dev/shm/sem.sem_pedidos_*`.

---

## 3. 🔍 Análisis frente a Problemas

### a. Prevención de Race Conditions Identificadas
- **Lectura Inconsistente / Escritura Parcial**: Se evita mediante la exclusión mutua de `sem_mutex`. Ningún proceso puede leer mientras otro está escribiendo una línea en `pedidos.txt`.
- **EOF Prematuro**: En la versión sin sincronización, la cocina leía hasta `EOF` y terminaba. Con `sem_wait(sem_items)`, la cocina queda suspendida en el kernel hasta que la recepción notifique un nuevo pedido con `sem_post(sem_items)`.
- **Numeración Duplicada por Múltiples Recepciones**: El cálculo del número del pedido entrante se realiza dentro del bloque protegido por `sem_mutex`, garantizando un orden secuencial estricto.

### b. Comportamiento si la Cocina está Caída
Si la recepción genera pedidos mientras la cocina no se encuentra en ejecución:
- La recepción escribe los pedidos en `pedidos.txt` e incrementa el contador del semáforo `/sem_pedidos_items`.
- Al iniciar la cocina posteriormente, `sem_open` recupera el valor acumulado en `sem_items` (o cuenta las líneas existentes), consumiendo de inmediato todos los pedidos pendientes acumulados de manera uniforme.

---

## 4. ⚠️ Limitaciones Conocidas

1. **Muerte de un Proceso dentro de la Sección Crítica**:
   Si un proceso es finalizado abruptamente (`SIGKILL` / `kill -9` o fallo por error de segmento `SIGSEGV`) mientras mantiene tomado el semáforo `sem_mutex`, el semáforo quedará en estado bloqueado de forma indefinida (*deadlock*). La API de semáforos POSIX nombrados no cuenta con un mecanismo de cerrojo robusto (*robust mutex*) que libere automáticamente el semáforo si el dueño fallece.
2. **Semáforos Huérfanos por `SIGKILL`**:
   Si el proceso es interrumpido con `SIGKILL`, los manejadores de señales no se ejecutan y el semáforo permanecerá guardado en `/dev/shm/`. Para solucionar esto de manera externa se requiere ejecutar `make clean` o remover `/dev/shm/sem.sem_pedidos_*` manualmente.

---

## 🛠️ Compilación y Ejecución

Compilar el proyecto:
```bash
make clean && make
```

Ejecutar la cocina en una terminal (esperará pedidos de forma bloqueante):
```bash
./cocina
```

En otra terminal, enviar pedidos desde la recepción:
```bash
./recepcion 5
```

Limpiar binarios, temporales y semáforos:
```bash
make clean
```
