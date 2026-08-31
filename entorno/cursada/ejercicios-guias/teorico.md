### **a) Comportamiento observado**

* **Primer lote con `./cocina`**: Al ejecutar `./cocina` por primera vez, la función `leer_proximo_pedido()` abre `pedidos.txt` en modo lectura (`"r"`), lee e imprime secuencialmente cada pedido registrado desde el inicio hasta alcanzar el final del archivo (`EOF`), cerrando el archivo al finalizar. Todos los pedidos del primer lote se procesan correctamente.
* **Segunda ejecución de `./recepcion 3`**: Los pedidos nuevos **se agregan (append)** a continuación de los existentes (manteniendo la numeración incremental, p. ej. `PEDIDO-006`, `PEDIDO-007`, `PEDIDO-008`).
* **¿Por qué?**: En `pedidos.c`, la función `registrar_pedido()` abre el archivo en modo **append** (`"a"`). Este modo posiciona el puntero de escritura automáticamente al final de `pedidos.txt`, preservando el contenido anterior sin sobrescribirlo. Además, la función `contar_pedidos_existentes()` lee los pedidos previos para continuar la numeración consecutiva.

---

### **b) Identificación de procesos**

* **Durante `./recepcion 5`**: Había **1 solo proceso** en ejecución en el sistema correspondiente al ejecutable `./recepcion`.
* **Durante `./cocina`**: Había **1 solo proceso** en ejecución en el sistema correspondiente al ejecutable `./cocina`.

> **Explicación**: Ambos programas se ejecutan de manera secuencial e independiente como procesos individuales en el espacio de usuario. Ninguno de ellos crea procesos hijos (`fork()`) ni se ejecutan en segundo plano al mismo tiempo.

---

### **c) Ventajas y desventajas de la arquitectura modular**

#### **Ventajas concretas**:
1. **Reutilización de código (DRY)**: La lógica de manejo de archivos y formateo (`registrar_pedido`, `leer_proximo_pedido`, `formatear_linea`, `contar_pedidos_existentes`) se centraliza en la librería compartida/módulo `pedidos.c/h`, evitando duplicar código.
2. **Separación de responsabilidades (SRP)**: `recepcion.c` se encarga únicamente de generar e ingresar pedidos, mientras que `cocina.c` se ocupa exclusivamente de procesarlos.
3. **Mantenibilidad y modularidad**: Es posible modificar la implementación interna de la recepción o la cocina sin afectar el otro ejecutable, manteniendo estable la interfaz en `pedidos.h`.
4. **Compilación incremental eficiente**: Si solo se cambia `cocina.c`, únicamente se recompila dicho archivo y se enlaza con `pedidos.o`, sin necesidad de recompilar `recepcion.c`.

#### **Desventajas**:
1. **Mayor complejidad de gestión de proyecto**: Requiere organizar múltiples archivos fuente (`.c`, `.h`), objetos (`.o`), ejecutables y un `Makefile` para controlar las dependencias.
2. **Aislamiento de memoria**: Al ser procesos distintos con espacios de direcciones de memoria separados por el SO, no pueden compartir variables directamente y dependen obligatoriamente de un medio externo (como `pedidos.txt`) para transferir datos.

---

### **d) Compilación incremental con `make`**

* **Ejecutar `make` dos veces seguidas**:
  * La segunda vez `make` informa que los objetivos están actualizados (`make: Nothing to be done for 'all'`) y **no recompila nada**.
  * **Justificación**: `make` compara las marcas de tiempo (*timestamps*) de modificación de los archivos fuente (`.c`, `.h`) con los binarios de salida (`.o`, ejecutables). Al detectar que los binarios son más recientes que las fuentes, determina que no hay trabajo por hacer.

* **Modificar solo `recepcion.c` y volver a ejecutar `make`**:
  * **Archivos recompilados / ejecutados**:
    1. `gcc -std=c89 -pedantic -Wall -Werror -c recepcion.c` (recompila `recepcion.c` a `recepcion.o`).
    2. `gcc -std=c89 -pedantic -Wall -Werror -o recepcion recepcion.o pedidos.o` (revincula el ejecutable `recepcion`).
  * **Archivos NO recompilados**: `pedidos.c`, `pedidos.o`, `cocina.c`, `cocina.o` y el ejecutable `cocina`.
  * **Justificación basada en las dependencias del Makefile**:
    * En la regla `recepcion.o: recepcion.c pedidos.h`, al modificarse `recepcion.c`, su timestamp pasa a ser más reciente que `recepcion.o`, lo que fuerza su recompilación.
    * En la regla `recepcion: recepcion.o pedidos.o`, al actualizarse `recepcion.o`, se dispara la vinculación final del ejecutable `recepcion`.
    * Los objetivos `cocina.o`, `pedidos.o` y `cocina` no dependen de `recepcion.c`, por lo que `make` omite su compilación.

---

### **e) Concurrencia y acceso simultáneo al recurso compartido**

#### **Problemas al acceder simultáneamente al mismo archivo**:
1. **Condiciones de Carrera (Race Conditions)**: Si `recepcion` escribe mientras `cocina` intenta leer, `cocina` puede leer líneas incompletas o corruptas.
2. **Terminación prematura por EOF (End Of File)**: La función `leer_proximo_pedido()` retorna 0 cuando `fgets()` detecta `EOF`. Si la cocina lee más rápido de lo que la recepción escribe, `cocina` alcanzará el final del archivo, cerrará el puntero `f_lectura` y finalizará su ejecución, ignorando pedidos que entren instantes después.
3. **Buffering de E/S**: El sistema de entrada/salida de C utiliza buffers. Las escrituras de `recepcion` pueden no reflejarse de inmediato en disco para `cocina` si no se fuerza la descarga del buffer (`fflush`).

#### **¿Por qué separar el código en dos procesos no resuelve el problema en paralelo?**
Separar el código en dos procesos independientes les otorga espacios de memoria aislados, pero **no establece ningún mecanismo de sincronización ni comunicación entre ellos (IPC)**. Ambos procesos actúan a ciegas compitiendo por un recurso plano común sin coordinación.

#### **¿Qué faltaría para resolverlo adecuadamente?**
1. **Sincronización de archivos o exclusión mutua**: Bloqueos de archivo (`flock()` / `fcntl()`) o semáforos para garantizar acceso atómico y no simultáneo al archivo.
2. **Mecanismos de Comunicación entre Procesos (IPC)**: Utilizar tuberías (*Pipes* / *FIFOs*), *Sockets* o colas de mensajes (*Message Queues*) en lugar de un archivo de texto pasivo.

---

## **MEJORAS POSIX**

### **1) Recursos Compartidos**

A continuación se detallan los recursos que los procesos `recepcion` y `cocina` comparten actualmente o podrían compartir en una arquitectura POSIX mejorada:

1. **Archivo de pedidos (`pedidos.txt`)**
   - **Nombre del recurso**: Archivo plano en disco (`pedidos.txt`).
   - **Tipo de recurso**: Archivo en sistema de archivos (File System Persistent Resource).
   - **Quién lo lee y quién lo escribe**:
     - **Lectura**: `recepcion` (al inicio en `contar_pedidos_existentes()` para determinar el número consecutivo inicial) y `cocina` (en `leer_proximo_pedido()` para obtener los pedidos a preparar).
     - **Escritura**: `recepcion` (en `registrar_pedido()` en modo append `"a"` para añadir nuevas líneas de pedidos).

2. **Terminal / Consola de Salida (`stdout` / `stderr`)**
   - **Nombre del recurso**: Consola / Terminal estándar del sistema.
   - **Tipo de recurso**: Dispositivo E/S de caracteres (Character Device / TTY).
   - **Quién lo lee y quién lo escribe**:
     - **Escritura**: Tanto `recepcion` (`printf`, `fprintf`) como `cocina` (`printf`) escriben sus logs en pantalla.
     - **Lectura**: El manejador de terminal del SO / usuario.

3. **Memoria Compartida POSIX (`shm_open` / `mmap`)** *(Recurso potencial en mejoras POSIX)*
   - **Nombre del recurso**: Segmento de Memoria Compartida POSIX / Buffer Circular de Pedidos.
   - **Tipo de recurso**: Variable / Estructura en memoria RAM compartida entre procesos.
   - **Quién lo lee y quién lo escribe**:
     - **Escritura**: `recepcion` (escribe nuevos pedidos en las posiciones del buffer).
     - **Lectura**: `cocina` (lee y remueve pedidos del buffer).

4. **Semáforos / Mutexes POSIX (`sem_t` / `pthread_mutex_t`)** *(Recurso potencial en mejoras POSIX)*
   - **Nombre del recurso**: Semáforos POSIX de sincronización (`sem_pedidos_disponibles`, `sem_espacio_libre`, `mutex_archivo`).
   - **Tipo de recurso**: Objeto de Sincronización del SO (Kernel / Memory-mapped sync primitive).
   - **Quién lo lee y quién lo escribe**:
     - **Lectura/Escritura (Modificación de Estado)**: Ambos procesos (`recepcion` realiza `sem_post` / `pthread_mutex_unlock` y `cocina` realiza `sem_wait` / `pthread_mutex_lock`).

---

### **2) Secciones Críticas**

Las secciones críticas son aquellas zonas de código donde los procesos acceden al recurso compartido (`pedidos.txt`).

#### **En `recepcion.c` (y módulo `pedidos.c`)**:

1. **Sección Crítica 1: Lectura para Conteo Inicial de Pedidos**
   - **Ubicación en el código**: `main()` de `recepcion.c` (línea 36: `inicio_numero = contar_pedidos_existentes() + 1;`), que invoca a la función `contar_pedidos_existentes()` en `pedidos.c` (líneas 80–101).
   - **Recurso compartido**: Archivo `pedidos.txt`.
   - **Tipo de acceso**: **Lectura** (`fopen(..., "r")`, `fgets()`).

2. **Sección Crítica 2: Escritura y Registro del Pedido**
   - **Ubicación en el código**: Bucle `for` de `main()` en `recepcion.c` (línea 42: `registrar_pedido(numero_actual, desc_azar)`), que ejecuta la función `registrar_pedido()` en `pedidos.c` (líneas 25–48).
   - **Recurso compartido**: Archivo `pedidos.txt`.
   - **Tipo de acceso**: **Escritura** (`fopen(..., "a")`, `fprintf()`, `fclose()`).

#### **En `cocina.c` (y módulo `pedidos.c`)**:

1. **Sección Crítica 1: Lectura del Próximo Pedido**
   - **Ubicación en el código**: Condición de control del bucle `while` en `main()` de `cocina.c` (línea 11: `while (leer_proximo_pedido(&numero, descripcion))`), que ejecuta `leer_proximo_pedido()` en `pedidos.c` (líneas 50–78).
   - **Recurso compartido**: Archivo `pedidos.txt`.
   - **Tipo de acceso**: **Lectura** (`fopen(..., "r")` en la primera llamada, `fgets()`).

---

### **3) Race Conditions específicas**

#### **Race Condition 1: Duplicación e Inconsistencia en la Numeración de Pedidos**

Ocurre si dos instancias de `recepcion` se ejecutan en paralelo. La consulta del total existente y la escritura posterior no forman una operación atómica.

| Tiempo | Proceso A (`recepcion 1`) | Proceso B (`recepcion 2`) | Estado del recurso compartido (`pedidos.txt`) |
| :--- | :--- | :--- | :--- |
| $t_0$ | Inicia ejecución. | Inicia ejecución. | Contiene 5 pedidos (`PEDIDO-001` a `PEDIDO-005`). |
| $t_1$ | Llama a `contar_pedidos_existentes()`. Lee 5 pedidos y calcula `inicio_numero = 6`. | En espera / Inicio. | Contiene 5 pedidos (`PEDIDO-001` a `PEDIDO-005`). |
| $t_2$ | Prepara en RAM `PEDIDO-006: Hamburguesa`. | Llama a `contar_pedidos_existentes()`. Lee 5 pedidos y calcula `inicio_numero = 6`. | Contiene 5 pedidos (`PEDIDO-001` a `PEDIDO-005`). |
| $t_3$ | Llama a `registrar_pedido()`. Escribe `PEDIDO-006: Hamburguesa\n`. | Prepara en RAM `PEDIDO-006: Pizza`. | Contiene 6 pedidos (último: `PEDIDO-006: Hamburguesa`). |
| $t_4$ | Finaliza `registrar_pedido()`. | Llama a `registrar_pedido()`. Escribe `PEDIDO-006: Pizza\n`. | Contiene 7 pedidos pero con IDs duplicados (`PEDIDO-006` repetido). |

- **Resultado correcto sin intercalar**: Si `recepcion 1` termina de registrar su pedido antes de que `recepcion 2` cuente, `recepcion 1` genera `PEDIDO-006` y `recepcion 2` cuenta 6 pedidos e inicia en `PEDIDO-007`.
- **Resultado anómalo**: Se generan dos pedidos con el mismo número identificador (`PEDIDO-006`), violando la unicidad del número de pedido.
- **Por qué es posible**: Porque el conteo (`contar_pedidos_existentes`) y la escritura (`registrar_pedido`) están desacoplados sin un bloqueo de exclusión mutua (mutex o lock de archivo `flock`) que garantice la atomicidad de la transacción.

#### **Race Condition 2: Lectura Corrupta / Parcial por Escritura Intercalada**

Ocurre si `cocina` intenta leer mediante `fgets` mientras `recepcion` está escribiendo a medio camino en `pedidos.txt`.

| Tiempo | Proceso A (`recepcion`) | Proceso B (`cocina`) | Estado del recurso compartido (`pedidos.txt`) |
| :--- | :--- | :--- | :--- |
| $t_0$ | Abre `pedidos.txt` en modo `"a"`. | Llama a `leer_proximo_pedido()`, posicionado al final de las líneas previas. | Contiene `PEDIDO-001: Pizza\n`. |
| $t_1$ | Llama a `fprintf` escribiendo parcialmente `"PEDIDO-002: Ham"` (sin `\n` aún). | Interrupción de CPU / En espera. | Buffer de E/S del SO con texto incompleto `"PEDIDO-002: Ham"`. |
| $t_2$ | Interrupción por Quantum de CPU. | Llama a `fgets()`, lee la porción parcial `"PEDIDO-002: Ham"`. Llama a `sscanf`. | Archivo con línea sin salto de línea (`\n`). |
| $t_3$ | En espera. | `sscanf("PEDIDO-002: Ham", ...)` no logra matchear el formato esperado o parsea una descripción incompleta. `leer_proximo_pedido()` retorna 0 y `cocina` asume EOF, cerrando el archivo (`fclose`). | `cocina` descarta/falla la lectura y finaliza la ejecución. |
| $t_4$ | `recepcion` se reanuda, escribe `burguesa\n` y cierra el archivo. | Proceso `cocina` ha finalizado prematuramente. | Contiene `PEDIDO-001` y `PEDIDO-002: Hamburguesa\n`, pero `cocina` ya no está corriendo. |

- **Resultado correcto sin intercalar**: `recepcion` escribe la línea completa con `\n` y realiza `fflush`/`fclose`. Luego `cocina` lee la línea integra y parsea correctamente `numero=2` y `descripcion="Hamburguesa"`.
- **Resultado anómalo**: `cocina` lee una cadena incompleta, falla al parsear o interpreta prematuramente el fin de archivo (EOF), cerrándose antes de procesar el pedido.
- **Por qué es posible**: Las operaciones de Entrada/Salida en archivos mediante la librería estándar de C no son atómicas a nivel de línea si no existe sincronización previa. El sistema operativo puede suspender la ejecución de un proceso a mitad de una llamada de escritura en disco/buffer.

#### **Race Condition 3: Finalización Prematura de la Cocina por EOF Falso**

Ocurre cuando `cocina` consume pedidos más rápido de lo que `recepcion` tarda en escribir el siguiente en el bucle `for`.

| Tiempo | Proceso A (`recepcion`) | Proceso B (`cocina`) | Estado del recurso compartido (`pedidos.txt`) |
| :--- | :--- | :--- | :--- |
| $t_0$ | Escribe `PEDIDO-001` en `pedidos.txt` y cierra el archivo. | Llama a `leer_proximo_pedido()`, lee `PEDIDO-001`. | Contiene `PEDIDO-001`. |
| $t_1$ | En el bucle `for`, ejecuta operaciones en memoria (p. ej. `rand()`). | Procesa `PEDIDO-001` rápidamente e inmediatamente vuelve a llamar a `leer_proximo_pedido()`. | Contiene `PEDIDO-001`. |
| $t_2$ | Aún no ha llamado a `registrar_pedido()` para el `PEDIDO-002`. | `leer_proximo_pedido()` intenta leer de `pedidos.txt` y detecta EOF (`fgets` retorna `NULL`). | Contiene `PEDIDO-001`. |
| $t_3$ | En espera. | Llama a `fclose(f_lectura)`, setea `f_lectura = NULL` y retorna `0`. `cocina` **finaliza su bucle `while` y el programa termina (`EXIT_SUCCESS`)**. | Contiene `PEDIDO-001`. |
| $t_4$ | Llama a `registrar_pedido()` y escribe `PEDIDO-002`. | `cocina` se encuentra finalizada (proceso muerto). | Contiene `PEDIDO-001` y `PEDIDO-002`, pero `PEDIDO-002` **nunca fue procesado**. |

- **Resultado correcto sin intercalar**: `recepcion` escribe todos sus pedidos. Luego `cocina` inicia y procesa la lista completa de principio a fin.
- **Resultado anómalo**: `cocina` se apaga creyendo que no hay más pedidos, ignorando los nuevos pedidos que `recepcion` registra momentos después.
- **Por qué es posible**: No existe un canal IPC pasivo-activo (como tuberías o colas de mensajes) ni señalización de fin de datos; `cocina` interpreta la llegada al final del archivo (`EOF`) como una condición de parada definitiva.

---

## **Análisis temporal y Tiempo Real**

### **1) Clasificación del Sistema Actual**

**a) ¿El sistema de pedidos del restaurante (tal como lo planteamos hasta ahora) es un sistema de tiempo real? Justificar.**

**No, no es un sistema de tiempo real.**

* **Justificación**: Un sistema de tiempo real (STR) se caracteriza formalmente porque la corrección y validez del sistema depende no solo de la exactitud lógica del resultado computacional, sino también del **tiempo o plazo límite (*deadline*)** en el que dicho resultado se entrega. En el sistema actual, la validez del procesamiento de un pedido es estrictamente **funcional y lógica**: si la cocina lee un pedido y lo prepara en 1 segundo, 10 minutos o 3 horas, el programa completa su ejecución con éxito (`EXIT_SUCCESS`) y computacionalmente el pedido se considera válido. No se imponen restricciones ni garantías temporales a nivel de diseño ni de SO.

**b) Si no lo es, ¿qué características le faltan? Si lo es, ¿es duro o blando? ¿Por qué?**

Al no ser de tiempo real, al sistema le faltan las siguientes características clave:
1. **Plazos explícitos (*Deadlines*)**: Definición formal de límites temporales máximos tolerables para la llegada, consumo y finalización de cada pedido.
2. **Determinismo Temporal (Latencia Acotada / Bounded Latency)**: Capacidad de garantizar un tiempo de respuesta máximo predecible independientemente de la carga general del sistema.
3. **Planificación de Tiempo Real (Real-Time Scheduling)**: Uso de políticas de planificación del SO orientadas a tiempo real (como POSIX `SCHED_FIFO` o `SCHED_RR`) con prioridades fijas preemplitivas.
4. **Temporización y Timeouts POSIX**: Implementación de temporizadores del sistema (`timer_create`, `clock_gettime`, `select`/`poll` con timeout) para gestionar o descartar tareas que excedan su ventana temporal.

---

### **2) Variante hipotética con restricción temporal**

Regla de negocio adicional: *"Si un pedido permanece en el archivo más de 30 segundos sin ser tomado por la cocina, la comida que lleva se considera fría y debe descartarse, generando un costo para el restaurante."*

**a) ¿Esta variante convierte al sistema en uno de tiempo real? Justificar.**

**Sí, lo convierte en un sistema de tiempo real.**

* **Justificación**: Se introduce formalmente un **plazo estricto (*deadline*) de 30 segundos** entre la generación del evento (registro del pedido por recepción) y su atención por parte del sistema (toma por parte de la cocina). La corrección del sistema ahora está ligada a la variable tiempo: tomar un pedido a los 31 segundos representa un **fallo del sistema / fallo de negocio (comida fría y pérdida económica)**, aun cuando el código de parseo y preparación funcione impecablemente a nivel lógico.

**b) Si lo convierte, ¿es tiempo real duro o blando? Argumentar usando los criterios de POSIX y sistemas en tiempo real.**

Es un sistema de **Tiempo Real Blando (*Soft Real-Time*)**.

* **Argumentación**:
  - En un sistema de **Tiempo Real Duro (*Hard Real-Time*)**, el incumplimiento de un *deadline* produce un fallo catastrófico e inaceptable con consecuencias humanas o materiales graves (ej. control de frenos ABS, avionica, marcapasos o reactores nucleares).
  - En este sistema, superar los 30 segundos **degrada la calidad del servicio (*QoS*) y genera un costo económico (descarte de comida)**, pero no causa una catástrofe que ponga en riesgo la integridad física ni destruya el sistema informático. El restaurante puede continuar operando tras el fallo.
  - **Criterios POSIX**: Desde la perspectiva POSIX, sobre un SO convencional (Linux/macOS con scheduler CFS), se pueden usar temporizadores de tiempo real POSIX (`clock_gettime(CLOCK_MONOTONIC)`), pero el sistema operativo no ofrece determinismo temporal absoluto a nivel de microsegundos de un RTOS o kernel RT-PREEMPT duro, encajando conceptual y técnicamente dentro de Tiempo Real Blando.

**c) Si se tuviera que diseñar el sistema con esta restricción, ¿qué cambios se harían sobre el diseño de la Parte 2? Pensar en, al menos, dos cambios concretos.**

1. **Inclusión de Marca de Tiempo POSIX (*Timestamp*) en los Pedidos**:
   - Al registrar un pedido (`registrar_pedido`), la recepción capturará el tiempo actual mediante la API POSIX `clock_gettime(CLOCK_MONOTONIC, &ts)` o `time(NULL)` y lo incluirá en el formato de registro (`PEDIDO-001 | TIMESTAMP: 1700000000 | Hamburguesa`). La cocina, al leer el pedido (`leer_proximo_pedido`), calculará `tiempo_espera = tiempo_actual - timestamp`. Si `tiempo_espera > 30`, el pedido se marcará como vencido/frío y se descartará.

2. **Reemplazo del Archivo Plano por un Canal IPC Activo POSIX con Timeouts**:
   - Sustituir el archivo `pedidos.txt` por una **Cola de Mensajes POSIX (`mq_open`, `mq_timedreceive`)** o **Memoria Compartida con Semáforos con Timeout (`sem_timedwait`)**. De este modo, la cocina se bloquea activamente a la espera de eventos sin hacer polling en disco, y el sistema puede descartar o expirar automáticamente los mensajes que hayan permanecido en la cola más de 30 segundos sin consumirse.

3. **Configuración de Políticas de Planificación POSIX de Tiempo Real**:
   - Asignar al proceso `cocina` una política de planificación `SCHED_FIFO` o `SCHED_RR` mediante `sched_setscheduler()` con alta prioridad, garantizando que el planificador del SO le otorgue CPU de manera inmediata cuando ingresa un pedido, evitando retrasos por multitarea convencional.

---

### **3) Estrategia ante Fallos**

**a) ¿Convendría una estrategia fail-safe o fail-soft para este sistema? ¿Por qué?**

Convendría una estrategia **Fail-Soft (Degradación Grácil)**.

* **Justificación**:
  - Una estrategia **Fail-Safe** detiene por completo el sistema o lo lleva a un estado de bloqueo seguro ante el primer fallo para evitar cualquier daño. En un restaurante, detener toda la operación por completo porque algunos pedidos tardaron más de 30 segundos resultaría desproporcionado e inviable comercialmente.
  - Una estrategia **Fail-Soft** prioriza que el sistema **continúe operando aunque sea con una capacidad o nivel de servicio reducido (degradado)**. Ante ráfagas de alta demanda o demoras en la cocina, el sistema debe ser capaz de descartar los pedidos fríos, reordenar prioridades o limitar temporalmente la admisión de nuevos pedidos en recepción, permitiendo que el restaurante siga procesando los pedidos que sí pueden ser servidos a tiempo.

**b) Dar un ejemplo concreto de cómo se manifestaría esa estrategia en el comportamiento del sistema.**

* **Escenario de Fallo**: Alta ráfaga de pedidos entrantes que provoca la acumulación de pedidos en cola y la cocina no da abasto, comenzando a superar el plazo de 30 segundos.
* **Comportamiento Fail-Soft del Sistema**:
  1. **Cocina (Purga y Métricas de Degradación)**: Al leer un pedido con `tiempo_espera > 30s`, la cocina registra el evento de descarte (`[ALERTA] Pedido #008 descartado por frio (35s en cola)`), incrementa un contador de pérdidas y prosigue inmediatamente con el siguiente pedido en cola sin detenerse ni colapsar.
  2. **Recepción (Control de Admisión / Rate Limiting)**: La cocina envía una señal POSIX (`SIGUSR1`) o un mensaje de alerta a `recepcion` informando que la cola está saturada. La recepción reacciona ajustando dinámicamente su comportamiento: suspende temporalmente el ingreso de nuevos pedidos o incrementa el tiempo entre registros, informando al cliente que el servicio opera con demora, manteniendo el sistema activo y estabilizando el flujo de trabajo.

