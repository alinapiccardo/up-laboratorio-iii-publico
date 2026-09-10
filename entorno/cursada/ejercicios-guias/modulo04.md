# Módulo 04

Esta carpeta contiene una **solución de referencia** del Trabajo Práctico del Módulo 04. A diferencia de los otros
módulos, este TP es **100% análisis y diseño**: no hay código que compilar. La entrega es un documento conceptual sobre
el sistema del restaurante del M03.

Lo que sigue es un modelo del análisis que esperábamos. **No es la única respuesta válida**: muchos puntos admiten
matices y argumentos distintos. Lo importante es que tu análisis use el vocabulario formal de los apuntes (race
condition, sección crítica, exclusión mutua, progreso, espera acotada, tiempo real duro/blando, fail-safe, fail-soft) y
que justifiques tus decisiones.

## Resumen del análisis modelo

### Parte 1 - Identificación de problemas concurrentes

**Recursos compartidos:** el más evidente es el archivo `pedidos.txt`, escrito por `recepcion` (modo `"a"`) y leído por
`cocina` (modo `"r"`). Hay también recursos implícitos que valen la pena mencionar:

- El **terminal/stdout**: ambos procesos imprimen ahí (Fijate que al escribir ambos procesos en el mismo terminal, sus
  mensajes pueden mezclarse).
- El **estado de numeración** de los pedidos (si quisiéramos numeración única globalmente, sería un contador
  compartido).
- El **cursor de lectura** que mantiene `cocina` con una variable global.

**Secciones críticas:** en `recepcion`, el bloque completo de `registrar_pedido` (apertura + escritura + cierre). En
`cocina`, el `fgets` dentro de `leer_proximo_pedido`. Ambas operan sobre el mismo archivo.

**Race conditions identificadas** (al menos tres tipos distintos):

1. **Lectura inconsistente**: si la escritura de una línea no es atómica, `cocina` puede leer una línea a medias.
2. **EOF prematuro**: `cocina` llega al final del archivo, asume "no hay más pedidos" y termina, pero `recepcion`
todavía está produciendo. Los pedidos siguientes quedan en el archivo sin consumirse.
3. **Orden no determinístico / numeración duplicada**: dos `recepcion` simultáneas asignan los mismos números (001,
002...) y el orden final en el archivo depende del scheduling del SO.

Cada una de estas race conditions debería estar acompañada por una tabla paso a paso que muestre cómo se produce.

### Parte 2 - Análisis temporal y tiempo real

**El sistema actual NO es un sistema de tiempo real.** No hay un plazo especificado ni garantizado para que un pedido
sea atendido. Si la cocina tarda 1 ms o 1 hora en consumir un pedido, no pasa nada desde el punto de vista del diseño.

**La variante con regla de 30 segundos sí convierte al sistema en uno de tiempo real blando** (soft real-time). El plazo
ahora está especificado (30 s) y existe una penalidad si se viola (la comida se descarta y genera costo), pero **no es
catastrófico**: el sistema sigue funcionando, simplemente con degradación económica. No es tiempo real duro porque
ningún pedido vencido implica daño físico, riesgo de vida ni cese del servicio.

Cambios necesarios para soportar esa restricción:

- Cada pedido debe llevar un **timestamp** de ingreso.
- Las operaciones de espera deben tener **timeouts** (`sem_timedwait`, `mq_timedreceive`).
- Posiblemente una **política de prioridad** que favorezca los pedidos más viejos.
- Un mecanismo para **descartar y registrar** los pedidos vencidos.

**Estrategia frente a fallos:** para un restaurante (servicio comercial, no crítico para la vida), la estrategia natural
es **fail-soft**. Si la cocina se cae, conviene seguir aceptando pedidos o avisar al usuario que el servicio está
degradado, en lugar de detener todo el sistema. Un sistema fail-safe (parar todo al detectar un fallo) sería
desproporcionado para este dominio.

## Errores típicos a evitar

- **No usar el vocabulario formal**: decir "se rompe el archivo" en lugar de decir que "ocurre una race condition por
  escritura no atómica".
- **No justificar las decisiones**: no alcanza con "uso un semáforo"; hay que decir _por qué_ ese mecanismo y _qué
  garantías_ aporta.
- **Confundir rápido con tiempo real**: un sistema puede ser muy rápido y no ser de tiempo real. Lo que define tiempo
  real es el plazo **especificado y garantizado**.
- **No pensar en casos límite**: ¿qué pasa si un proceso muere dentro de la sección crítica? ¿Si el archivo no existe
  cuando arranca cocina? Pensar estos casos suele revelar problemas del diseño.

## Conexión con los módulos siguientes

El análisis que entregás en este TP es la **base** de los próximos trabajos prácticos; un buen análisis ahora hace que esas implementaciones sean casi mecánicas. Un análisis deficiente te obliga a "redescubrir" los problemas mientras programás, lo cual muchas veces cuesta el doble.
