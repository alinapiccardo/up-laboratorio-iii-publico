# Informe de Diseño y Decisiones Técnicas - Trabajo Final Integrador (Parte I)

**Asignatura:** Laboratorio III  
**Tema:** Sistema SCADA de Monitoreo Concurrente de Sensores  
**Estándar de Lenguaje:** ANSI C / C89 (`-std=c89 -pedantic -Wall -Wextra -Werror`)  

---

## 1. Introducción y Arquitectura del Sistema

El objetivo de la Parte I de este trabajo final integrador es diseñar y construir la capa de adquisición y procesamiento concurrente para un sistema de supervisión y adquisición de datos (SCADA). El sistema debe recolectar mediciones emitidas por múltiples sensores, procesarlas en paralelo manteniendo métricas en tiempo real y permitir la consulta concurrente desde un proceso cliente sin detener la recolección.

### Diagrama de Bloques del Sistema

```text
 ┌────────────────────────────────────────────────────────┐
 │                   PROCESO COLECTOR                     │
 │                                                        │
 │  ┌─────────────┐       ┌───────────────┐               │
 │  │ Worker 1    │──────>│               │               │
 │  ├─────────────┤       │ Memoria       │               │
 │  │ Worker 2    │──────>│ Compartida    │<───── (IPC) ──┼─── PROCESO CONSULTA
 │  ├─────────────┤       │ (shm_scada)   │               │   (Lectura en vivo)
 │  │ Worker N    │──────>│               │               │
 │  └─────────────┘       └───────┬───────┘               │
 │         ▲                      │                       │
 │         │ (mq_receive)  [sem_scada_mutex]              │
 └─────────┼──────────────────────────────────────────────┘
           │
 ┌─────────┴──────────────┐
 │ Cola de Mensajes POSIX │
 │ (mq_scada_mediciones)  │
 └─────────▲──────────────┘
           │ (mq_send)
 ┌─────────┴──────────────────────────────────────────────┐
 │                  PROCESOS SENSORES                     │
 │                                                        │
 │ ┌──────────┐      ┌──────────┐        ┌──────────┐     │
 │ │ Sensor 1 │      │ Sensor 2 │  ...   │ Sensor N │     │
 │ │ (fork)   │      │ (fork)   │        │ (fork)   │     │
 │ └──────────┘      └──────────┘        └──────────┘     │
 └────────────────────────────────────────────────────────┘
```

---

## 2. Justificación de las Decisiones de Diseño

### A. Uso de Procesos (`fork`) para los Sensores
* **Justificación:** Los sensores industriales son dispositivos físicamente independientes. En el sistema operativo, la abstracción natural para simular entidades independientes con espacio de memoria aislado es un **proceso**.
* **Beneficio:** Si un sensor sufre una falla o error grave, su espacio de memoria aislado evita que el Colector o los demás sensores se vean afectados.

### B. Comunicación vía Cola de Mensajes POSIX (`<mqueue.h>`)
* **Justificación:** Se adoptó una Cola de Mensajes POSIX con nombre (`/mq_scada_mediciones`) para el envío de mediciones desde los sensores hacia el Colector.
* **Ventajas frente a otros IPCs:**
  1. **Desacoplamiento Asincrónico:** El sensor envía el mensaje mediante `mq_send` y continúa inmediatamente sin esperar que el Colector lo procese.
  2. **Estructuración Nativa de Datos:** Permite transmitir directamente la estructura `struct Medicion` de forma atómica.
  3. **Control Automático de Back Pressure:** El kernel de Linux administra el buffer interno (`mq_maxmsg = 10`). Si la cola se llena, los sensores productores se bloquean automáticamente sin necesidad de codificar algoritmos de sondeo activo.

### C. Memoria Compartida POSIX (`<sys/mman.h>`) para Estadísticas
* **Justificación:** Las estadísticas agregadas (mínimo, máximo, promedio, cantidad y último valor) se almacenan en un segmento de memoria compartida (`/shm_scada_stats`).
* **Beneficio:** Permite que el programa independiente `consulta` acceda a los datos consolidados en tiempo real sin requerir copia de bytes ni interrumpir la ejecución de los worker threads del Colector.

### D. Exclusión Mutua con Semáforo POSIX con Nombre (`<semaphore.h>`)
* **Justificación:** La memoria compartida por sí sola no provee sincronización. Para evitar **condiciones de carrera (Race Conditions)** mientras varios *worker threads* escriben y el cliente `consulta` lee, se utiliza un semáforo binario POSIX (`/sem_scada_mutex`) inicializado en `1`.
* **Regla de Exclusión Mutua:** Toda modificación y lectura de la estructura `struct MemoriaCompartida` se realiza dentro de la **Sección Crítica** delimitada por `sem_wait` y `sem_post`.

### E. Pool de Worker Threads (`<pthread.h>`) en el Colector (Patrón Fan-Out)
* **Justificación:** Dentro del Colector se crea un conjunto configurable de hilos de ejecución (`pthread_create`).
* **Beneficio:** Los hilos comparten el espacio de direcciones del Colector y consumen de la cola de mensajes en paralelo (*fan-out*), maximizando el aprovechamiento de los núcleos de CPU del sistema con un costo de conmutación de contexto mínimo.

