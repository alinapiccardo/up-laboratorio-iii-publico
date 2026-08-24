# Sistema de Catálogo de Biblioteca (`catalogo`)

Este proyecto es una aplicación en lenguaje C desarrollada para la asignatura **Laboratorio III**. Implementa un sistema completo de registro y gestión de libros de biblioteca en consola de comandos, utilizando archivos de texto para almacenamiento persistente.

---

## 🎯 Objetivos del Proyecto

- **Manejo de Argumentos (`argc` / `argv`)**: Interfaz flexible mediante comandos por consola o modo interactivo por menú.
- **Persistencia de Archivos**: Lectura y escritura con buffer mediante funciones estándar de C (`fopen`, `fgets`, `fprintf`, `fclose`).
- **Manipulación de Cadenas**: Procesamiento de líneas, delimitadores y búsquedas por subcadena insensibles a mayúsculas/minúsculas.
- **Arquitectura Modular**: Separación clara entre archivos de cabecera (`.h`), implementación (`.c`) y programa principal.
- **Compilación Incremental**: `Makefile` estructurado con variables y flags de compilación estrictos (`-std=c89 -pedantic -Wall -Wextra -Werror`).

---

## 📁 Estructura del Proyecto

```text
catalogo/
├── catalogo.h      # Declaración de estructuras (Libro) y prototipos de funciones
├── catalogo.c      # Implementación de la lógica de catálogo y persistencia
├── main.c          # Punto de entrada, procesamiento de argumentos y menú interactivo
├── Makefile        # Script de compilación incremental y pruebas
└── README.md       # Documentación general del proyecto
```

---

## 🛠️ Compilación y Ejecución

### Compilar el Proyecto
Para generar el ejecutable `catalogo`:
```bash
make
```

### Ejecutar Pruebas Automatizadas
Para recompilar y ejecutar una secuencia de pruebas básicas (agregar, listar y buscar):
```bash
make test
```

### Limpiar Archivos Generados
Para eliminar archivos objeto (`.o`), el ejecutable `catalogo` y el archivo de datos `catalogo.txt`:
```bash
make clean
```

---

## 🚀 Guía de Uso

El programa admite dos modos de funcionamiento: **Línea de Comandos (CLI)** y **Modo Interactivo**.

### 1. Uso mediante Argumentos por Consola (CLI)

#### ➕ Agregar un Libro
Agrega un nuevo libro generando automáticamente un ID aleatorio único de 5 dígitos:
```bash
./catalogo agregar "El Aleph" "Jorge Luis Borges"
./catalogo agregar "Cien años de soledad" "Gabriel García Márquez"
```

#### 📋 Listar Libros
Muestra en consola la lista completa de libros registrados en `catalogo.txt` formateados en tabla:
```bash
./catalogo listar
```

#### 🔍 Buscar Libros
Busca cualquier libro cuyo **título** o **autor** contenga el texto especificado (búsqueda por subcadena insensible a mayúsculas/minúsculas). Al finalizar la tabla, reporta el total de coincidencias encontradas:
```bash
./catalogo buscar "Borges"
./catalogo buscar "soledad"
```

#### ❓ Ayuda
Muestra la guía de uso de comandos:
```bash
./catalogo ayuda
```
o
```bash
./catalogo -h
```

---

### 2. Modo Interactivo
Si el programa se ejecuta sin argumentos:
```bash
./catalogo
```
Se desplegará un menú interactivo en consola:
```text
=======================================
      SISTEMA DE CATÁLOGO DE LIBROS    
=======================================
1. Agregar un libro
2. Listar libros
3. Buscar libros
4. Salir
---------------------------------------
Seleccione una opción [1-4]:
```

---

## 📄 Formato del Archivo de Datos (`catalogo.txt`)

Los libros se almacenan de manera persistente en `catalogo.txt` en el siguiente formato por línea, delimitado por punto y coma (`;`):

```text
<ID>;<Título>;<Autor>
```

**Ejemplo de contenido de `catalogo.txt`:**
```text
48291;El Aleph;Jorge Luis Borges
19384;Cien años de soledad;Gabriel García Márquez
72810;Rayuela;Julio Cortázar
```

---

## ⚙️ Criterios de Evaluación Cumplidos

1. **Compilación sin errores ni warnings**: Compilado estrictamente con `-std=c89 -pedantic -Wall -Wextra -Werror`.
2. **Separación `.h` / `.c`**: Módulo `catalogo` independizado para ser reutilizado.
3. **Manejo de `argc` / `argv`**: Parseo robusto de opciones, validación de cantidad de argumentos y mensajes de error descriptivos en `stderr`.
4. **Manejo correcto de archivos**: Apertura en modos seguros (`"a"` y `"r"`), verificación de punteros `NULL` y cierre garantizado con `fclose`.
5. **Manipulación de cadenas**: Función personalizada `contiene_subcadena_ci` para búsqueda *case-insensitive* portátil en C89/C99.
6. **Makefile con compilación incremental**: Reglas explícitas de dependencias entre `.o`, `.c` y `.h`.
