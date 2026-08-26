### **a) Describir el comportamiento observado: ¿en qué orden se ejecutan las operaciones?**

El comportamiento es **estrictamente secuencial, síncrono y por lotes (batch)**:

1. **Fase 1 (Recepción de pedidos)**: Se ejecuta la función `recibir_pedidos()` de principio a fin. Abre el archivo en modo escritura (`"w"`), genera los 10 pedidos completos, los escribe en disco y cierra el archivo (`fclose`).
2. **Fase 2 (Preparación de pedidos)**: Una vez que finalizó por completo la fase de recepción, se inicia la función `preparar_pedidos()`. Esta abre el archivo en modo lectura (`"r"`), lee secuencialmente desde el primer hasta el último pedido y finaliza cerrando el archivo.

> **Conclusión**: Las operaciones no se solapan en el tiempo. La cocina no procesa nada hasta que la recepción haya guardado el 100% de los pedidos.

---

### **b) Comparación con la realidad: ¿el programa simula correctamente cómo funcionaría un restaurante de delivery real? Justificar.**

**No, no lo simula correctamente.**
* En un restaurante real, el proceso es **asíncrono y concurrente**: a medida que un cliente realiza un pedido, la cocina lo recibe e inicia su preparación de inmediato, sin esperar a que finalice la jornada o a que entren 10 pedidos más.
* En este programa, si el primer cliente pide una hamburguesa (PEDIDO-001), su comida no entra a la cocina hasta que el cliente 10 haya terminado de pedir. En la realidad, esto provocaría tiempos de espera inaceptables y comida fría o demorada.

---

### **c) Escenario hipotético: imaginar que, mientras la cocina está preparando el pedido 002, llega un cliente nuevo con un pedido urgente. ¿El programa permite registrar ese pedido mientras la cocina trabaja? ¿Por qué?**

**No, el programa no lo permite.**

**Razones técnicas**:
1. **Hilo de ejecución único (Monohilo / Single-threaded)**: El programa ejecuta una sola instrucción a la vez de forma bloqueante. Mientras el procesador se encuentra ejecutando el bucle `while(fgets(...))` dentro de `preparar_pedidos()`, el hilo de ejecución está ocupado y no hay otro hilo ni proceso escuchando o ejecutando `recibir_pedidos()`.
2. **Modo de apertura de archivos**: Durante la lectura, el archivo fue abierto en modo lectura (`"r"`). Para agregar un nuevo pedido en ese instante se requeriría abrir el archivo en modo *append* (`"a"`) desde otra rutina simultánea, pero al no haber concurrencia/multiprocesamiento, no hay forma de interrumpir la lectura en tiempo real para escribir.

---

### **d) Identificación del problema: ¿cuál es la limitación fundamental de este enfoque? ¿Qué característica de un sistema real no puede representarse con esta implementación?**

#### 1. Limitación fundamental
La **ausencia de concurrencia** y el uso de **procesamiento por lotes rígido (Batch Processing)** mediante archivos de texto secuenciales sin sincronización.

#### 2. Características de un sistema real que no pueden representarse con este enfoque:
* **Concurrencia y paralelismo (Productor-Consumidor en tiempo real)**: En la realidad, la recepción actúa como *Productor* y la cocina como *Consumidor*. Ambos operan en paralelo y comparten una cola de trabajo.
* **Manejo de eventos e interrupciones en tiempo real**: No se pueden procesar pedidos entrantes de forma reactiva o prioritaria (ej. pedidos urgentes o cancelaciones) mientras se está ejecutando la lectura/preparación.
* **Sincronización de memoria compartida o IPC (Inter-Process Communication)**: Para lograr este comportamiento en sistemas informáticos reales se utilizan herramientas avanzadas como *threads* (hilos), *procesos con memoria compartida/sockets/pipes* y mecanismos de sincronización (*mutexes* o *semáforos*) en lugar de simples archivos de texto leídos de forma secuencial.