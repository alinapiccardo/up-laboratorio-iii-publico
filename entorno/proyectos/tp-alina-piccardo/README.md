# Trabajo Final Integrador - Parte I: Sistema SCADA

**Asignatura:** Laboratorio III  
**Modalidad:** Individual  
**Estándar de Compilación:** C89 / ANSI C estricto (`-std=c89 -pedantic -Wall -Wextra -Werror`)

---

## 1. Descripción del Proyecto

Este sistema simula la capa de adquisición y procesamiento de datos en tiempo real de una planta industrial SCADA:

1. **Sensores (Procesos Creados con `fork`):** Se crean $N$ procesos independientes. Cada uno simula lecturas continuas y las envía de forma desacoplada a través de una **Cola de Mensajes POSIX** (`/mq_scada_mediciones`).
2. **Procesamiento Concurrente (Worker Threads):** El proceso Colector dispone de un pool de hilos (`pthread_create`) que consumen concurrentemente las mediciones de la cola (*pattern fan-out*).
3. **Estadísticas en Memoria Compartida:** Los workers actualizan en tiempo real métricas agregadas por sensor y globales (mínimo, máximo, promedio, cantidad y último valor) en un segmento de **Memoria Compartida POSIX** (`/shm_scada_stats`), protegida mediante un **Semáforo POSIX con Nombre** (`/sem_scada_mutex`).
4. **Cliente de Consulta Independiente (`consulta`):** Un segundo proceso ejecuta una lectura atómica sobre la Memoria Compartida en tiempo real sin frenar al Colector.
5. **Terminación Limpia:** Al procesar las mediciones esperadas ($N \times M$), el sistema destruye ordenadamente los hilos, recolecta los procesos sensores (`waitpid`) y desvincula todos los recursos IPC (`mq_unlink`, `shm_unlink`, `sem_unlink`).

---

## 2. Estructura de Archivos

* `comun.h`: Definición de estructuras de datos (`struct Medicion`, `struct MemoriaCompartida`), límites del sistema y constantes de recursos IPC.
* `colector.c`: Proceso principal. Parsea argumentos de consola (`argv`), inicializa IPCs, realiza el `fork()` de sensores, administra el pool de worker threads y destruye los recursos al finalizar.
* `consulta.c`: Proceso lector independiente que se conecta a la memoria compartida y muestra el estado actual del sistema en consola.
* `Makefile`: Script de compilación estricta en C89.

---

## 3. Instrucciones de Compilación y Ejecución

### Opción A: Compilación en el Entorno Linux / Docker de la Materia (Recomendado)

Desde la raíz de la materia (`/entorno`):

```bash
# 1. Abrir la terminal dentro del contenedor Linux de la materia
make shell

# 2. Navegar a la carpeta del proyecto
cd proyectos/tp-alina-piccardo

# 3. Compilar el proyecto con el Makefile C89
make
```

### Opción B: Ejecución del Sistema

#### 1. Ejecutar el Colector
El colector recibe al menos dos argumentos por línea de comandos:  
`./colector <cant_sensores> <mediciones_por_sensor> [num_workers]`

Ejemplo (4 sensores, 30 mediciones por sensor, 2 worker threads):
```bash
./colector 4 30 2
```

#### 2. Ejecutar el Cliente de Consulta en Tiempo Real
En una **segunda terminal** (mientras el Colector se encuentra procesando):

```bash
./consulta
```

---

## 4. Limpieza de Recursos IPC

Si la ejecución se interrumpe prematuramente con `Ctrl+C`, se puede forzar la eliminación de los recursos IPC huérfanos con el comando:

```bash
make ipc_clean
```

O eliminándolos directamente en la consola de Linux:
```bash
rm -f /dev/shm/shm_scada_stats /dev/shm/sem_scada_mutex /dev/mqueue/mq_scada_mediciones
```
