# Manual de Usuario — net (Simulador/Analizador de Topologías de Red)

Índice
1. Visión general
2. Requisitos y preparación
3. Iniciar el programa
4. Interfaz de usuario (CLI)
5. Comandos disponibles (síntaxis, descripción y ejemplos)
   - nuevo-disp
   - conectar-dispositivo
   - guardar / cargar-grafo
   - ver-grafo
   - visualizar-grafo
   - ping
   - traceroute
   - fallar-enlace
   - analizar-resiliencia
   - optimizar-ruta
   - limpiar
   - ayuda / salir
6. Formato del archivo de topología (txt/topologia.txt)
7. Conceptos operativos y comportamiento interno (lo que hace el programa)
   - Vértices y aristas
   - Estado activo/fallido
   - Métricas de enlace
   - Algoritmos utilizados (visión general)
8. Ejemplos de uso paso a paso
   - Construir una topología mínima
   - Probar conectividad y analizar rutas
   - Simular fallos y evaluar resiliencia
   - Probar recomendaciones de optimización
9. Buenas prácticas y consejos
10. Errores comunes y solución de problemas
11. Limitaciones y comportamiento esperado
12. Preguntas frecuentes (FAQ)
13. Créditos y licencias

---

1. Visión general
-----------------
Este programa es una utilidad de línea de comandos para crear, editar, analizar y simular topologías de red representadas como un grafo dirigido. Permite:

- Añadir dispositivos (nodos) con nombre, IP, tipo y capacidad.
- Conectar dispositivos mediante enlaces (aristas) con latencia, ancho de banda y fiabilidad.
- Guardar y cargar topologías en/desde archivos de texto.
- Visualizar la topología en consola y (opcionalmente) con un visualizador gráfico externo.
- Ejecutar pruebas de conectividad simuladas: ping y traceroute (K rutas aproximadas).
- Simular fallos de enlaces y evaluar el impacto sobre la conectividad global.
- Sugerir optimizaciones de red (enlaces hipotéticos) para mejorar latencias.
- Analizar resiliencia para identificar nodos críticos y recomendaciones de redundancia.

El objetivo es servir tanto para fines didácticos (comprender conceptos de routing y resiliencia) como para apoyar decisiones de diseño simple de topologías.

---

2. Requisitos y preparación
---------------------------
Requisitos mínimos:
- Entorno POSIX (Linux, macOS). Hay código con condicionales para Windows, pero el uso de visualización gráfica depende de python3 y herramientas externas.
- Compilador C (si va a compilar desde fuentes).
- Python 3 (opcional, para la utilidad de visualización gráfica `src/verGrafoL.py`).
- Acceso a la terminal y permisos para leer/escribir archivos donde se guarden topologías.

Archivos importantes:
- Ejecutable principal (por ejemplo, `net` o `main` según compilación).
- Archivo de persistencia por defecto: `txt/topologia.txt` (si existe, se carga al iniciar).
- Script de visualización (opcional): `src/verGrafoL.py` (llamado con `visualizar-grafo`).

Preparación:
- Asegúrese de tener el directorio de trabajo con permisos de lectura/escritura.
- Si desea que el programa cargue una topología por defecto, cree `txt/topologia.txt` con el formato adecuado (ver sección 6).

---

3. Iniciar el programa
----------------------
Arranque en la terminal:

- Ejecute el binario compilado (por ejemplo `./net`).
- Al iniciar, el programa intenta cargar `txt/topologia.txt`. Si no existe, empieza con un grafo vacío.
- Obtendrá un prompt interactivo:
  net>

El programa espera comandos en texto; finalice con Enter. Para salir, use el comando `salir`.

Nota: algunas operaciones (visualizador gráfico) lanzan un proceso hijo que se mantiene hasta que el usuario sale o se termine el visualizador.

---

4. Interfaz de usuario (CLI)
----------------------------
- La interacción principal es por comandos simples, con argumentos separados por espacios.
- Los nombres de dispositivos no deben contener espacios (se tratan como tokens).
- Las direcciones IP deben estar en formato IPv4 válido (a.b.c.d con 0-255).
- Muchos comandos guardan automáticamente la topología en `txt/topologia.txt` después de operaciones que la modifican.

Algunos comandos imprimen mensajes de estado `[OK]`, `[ERROR]`, `[INFO]`, `[RESILIENCE]`, `[PING]`, `[TRACEROUTE]`, etc., para facilitar el seguimiento.

---

5. Comandos disponibles (síntaxis, descripción y ejemplos)
---------------------------------------------------------

Nota general: en la descripción se usan nombres de ejemplo como A, B, router1, host1, etc. Sustituya por los nombres de sus nodos.

- nuevo-disp <nombre> <ip> <tipo> <cap>
  - Descripción: Añade un nuevo dispositivo (vértice) al grafo.
  - Parámetros:
    - nombre: identificador único del dispositivo (sin espacios).
    - ip: dirección IPv4 válida.
    - tipo: uno de los valores soportados (router, switch, host, servidor). Si no se reconoce se asigna tipo por defecto.
    - cap: capacidad de procesamiento (entero, uso interno).
  - Ejemplo: nuevo-disp router1 192.168.1.1 router 100
  - Comportamiento: valida IP, evita duplicados, agrega vértice activo y guarda topología por defecto.

- conectar-dispositivo <origen> <destino> <lat> <bw> <fiab>
  - Descripción: Añade una arista dirigida desde origen a destino con métricas.
  - Parámetros:
    - origen, destino: nombres de nodos existentes.
    - lat: latencia en ms (entero ≥ 0).
    - bw: ancho de banda en Mbps (entero ≥ 0).
    - fiab: fiabilidad (double entre 0.0 y 1.0).
  - Ejemplo: conectar-dispositivo router1 host1 15 100 0.98
  - Comportamiento: comprueba existencia de nodos, crea enlace activo y guarda topología.

- guardar <nombre_archivo>
  - Descripción: Guarda la topología actual en un archivo de texto.
  - Parámetros:
    - nombre_archivo: ruta/archivo donde escribir.
  - Ejemplo: guardar backups/topo1.txt
  - Formato: legible por `cargar-grafo` (ver sección 6).

- cargar-grafo <nombre_archivo>
  - Descripción: Carga una topología desde un archivo (anula lo cargado en memoria).
  - Parámetros:
    - nombre_archivo: archivo a leer.
  - Ejemplo: cargar-grafo txt/topologia.txt
  - Comportamiento: parsea nodos y aristas siguiendo el formato esperado; reporta referencias inválidas en STDERR.

- ver-grafo
  - Descripción: Imprime en consola la lista de nodos y sus enlaces con sus métricas y estados.
  - Ejemplo: ver-grafo
  - Salida típica: lista enumerada de vértices con sus aristas detalladas.

- visualizar-grafo
  - Descripción: Lanza un visualizador gráfico externo (si está disponible). Esto se realiza con fork/exec de `python3 src/verGrafoL.py`.
  - Requisitos: `python3` y el script de visualización presente y funcional.
  - Comportamiento: inicia proceso hijo y lo mantiene en ejecución; al cerrar el programa, intenta terminar el visualizador.

- ping <origen> <destino> [count]
  - Descripción: Simula una serie de pings desde origen a destino sobre la ruta de menor costo (según latencia).
  - Parámetros:
    - origen, destino: nodos existentes.
    - count: opcional, número de pruebas (por defecto 4).
  - Ejemplo: ping host1 servidor1 5
  - Comportamiento:
    - Calcula la ruta por Dijkstra (minimiza latencia).
    - Para cada intento simula paso por cada enlace; en cada salto la arista puede fallar según su fiabilidad (probabilidad).
    - Imprime por intento si hubo respuesta y el tiempo de ida y vuelta aproximado (sumatoria de latencias de enlaces).
    - Muestra estadísticas: transmitidos, recibidos, % pérdida y rtt min/avg/max.

- traceroute <origen> <destino> [K]
  - Descripción: Busca hasta K rutas aproximadas entre origen y destino e imprime métricas agregadas.
  - Parámetros:
    - K: número de rutas a encontrar (por defecto 3).
  - Ejemplo: traceroute A B 4
  - Comportamiento:
    - Utiliza una aproximación a K-shortest que desactiva temporalmente aristas del mejor camino para encontrar alternativas.
    - Para cada ruta calcula: número de saltos, latencia total, ancho de banda mínimo y fiabilidad compuesta.
    - Imprime y muestra las rutas encontradas.

- fallar-enlace <origen> <destino>
  - Descripción: Marca la arista origen->destino como inactiva (simula caída).
  - Ejemplo: fallar-enlace router1 host1
  - Comportamiento: cambia el campo `activo` de esa arista a 0; guarda la topología.

- analizar-resiliencia
  - Descripción: Analiza impacto de fallos de nodos en la conectividad global y sugiere enlaces para mejorar resiliencia.
  - Comportamiento:
    - Cuenta cuántos nodos alcanzables hay desde un nodo activo de inicio.
    - Para cada nodo, simula su fallo (marcando inactivo) y cuantifica la pérdida de nodos alcanzables.
    - Identifica el nodo con mayor impacto (nodo crítico).
    - Sugiere conectividad redundante entre vecinos del nodo crítico si procede.
  - Salida: informe con impacto por nodo, nodo crítico identificado y sugerencias de conexiones.

- optimizar-ruta <origen> <destino>
  - Descripción: Heurística que evalúa el efecto de añadir hipotético enlace directo (con parámetros prefijados) para mejorar la latencia.
  - Comportamiento:
    - Comprueba si ya existe enlace directo.
    - Calcula latencia actual por Dijkstra.
    - Añade temporalmente un enlace hipotético con latencia baja/alta fiabilidad, recalcula y mide la mejora.
    - El enlace temporal se elimina después de la prueba.
    - Si la mejora supera el umbral (ej. 20% de mejora de latencia), recomienda añadir el enlace con sus parámetros.
  - Ejemplo: optimizar-ruta router1 servidor1

- limpiar
  - Descripción: Limpia la pantalla/terminal (comando equivalente `clear` o `cls`).
  - Ejemplo: limpiar

- ayuda
  - Descripción: Muestra un resumen de comandos y usos.

- salir
  - Descripción: Finaliza el programa. Si hay un visualizador gráfico en ejecución, el programa intentará terminarlo ordenadamente.

---

6. Formato del archivo de topología (txt/topologia.txt)
-------------------------------------------------------
El programa carga y guarda topologías en un formato textual simple (legible). Componentes principales:

- Una línea que indica número de nodos: `NS <n>`
- Varias líneas de nodos con prefijo `N`:
  - Formato: `N <nombre> <ip> <tipo_int> <cap>`
  - tipo_int es el entero que representa Tipo_Dispositivo (interno).
- Una línea que indica número de aristas: `AS <m>`
- Varias líneas de aristas con prefijo `A`:
  - Formato: `A <origen> <destino> <lat> <bw> <fiab> <activo>`
  - Donde:
    - origen / destino: nombres de nodos tal y como se guardaron.
    - lat: latencia (ms)
    - bw: ancho de banda (Mbps)
    - fiab: fiabilidad (float)
    - activo: 1 active, 0 inactivo

Ejemplo sencillo (representación conceptual):
N router1 192.168.1.1 0 100
N host1 192.168.1.2 2 10
A router1 host1 15 100 0.98 1

El cargador ignora líneas en blanco o comentarios que empiecen por `#`. Si una arista referencia un nodo inexistente, se reporta a stderr y se omite la arista.

---

7. Conceptos operativos y comportamiento interno
-----------------------------------------------
- Vértices (nodos): representan dispositivos (router, switch, host, servidor). Cada vértice tiene:
  - nombre, IP, tipo, capacidad de procesamiento (atributo informativo), estado `activo` (1) o `fallido` (0), lista de aristas salientes.

- Aristas (enlaces): son dirigidas y almacenan:
  - índice de destino, latencia_ms, ancho_banda_mbps, fiabilidad (probabilidad de que el paquete pase), estado `activo`/`fallado`.

- Estado: nodos y aristas pueden ser marcados inactivos para simular fallos.

- Métricas:
  - Latencia total de una ruta: sumatorio de latencias de enlaces que la componen.
  - Ancho de banda mínimo: el mínimo ancho_banda_mbps entre enlaces de la ruta.
  - Fiabilidad compuesta: producto de las fiabilidades de los enlaces (modelo simplificado).

- Algoritmos:
  - Camino mínimo: se usa Dijkstra sobre el grafo, con función de coste configurable (en la práctica, se usa latencia).
  - K-rutas aproximadas: se busca la mejor ruta y luego se "desactiva" aristas del mejor camino para encontrar rutas alternativas (heurística aproximada).
  - Ping: usa la ruta calculada por Dijkstra y simula pérdidas por probabilidad (comparando rand() con la fiabilidad de cada enlace).
  - Análisis de resiliencia: cuenta nodos alcanzables con BFS simple (ignora nodos/aristas inactivos), luego simula fallos de cada nodo y evalúa impacto.

- Guardado/carga: se almacenan tanto nodos como aristas y sus atributos y estados para persistencia.

---

8. Ejemplos de uso paso a paso
------------------------------

Ejemplo A: Crear y verificar una topología mínima
1. nuevo-disp R1 10.0.0.1 router 200
2. nuevo-disp H1 10.0.0.2 host 10
3. conectar-dispositivo R1 H1 10 100 0.99
4. ver-grafo
   - Debería mostrar R1 con una arista a H1 (lat=10ms, bw=100, fiab=0.99).

Ejemplo B: Ping simulado
1. ping R1 H1 4
   - Calcula ruta R1 -> H1; simula 4 paquetes; imprime resultados y estadísticas.

Ejemplo C: Buscar varias rutas (traceroute)
1. traceroute A B 3
   - Imprime hasta 3 rutas distintas (si existen) con métricas por ruta.

Ejemplo D: Simular fallo de enlace y analizar resiliencia
1. fallar-enlace R1 H1
2. ver-grafo
   - Arista R1 -> H1 mostrada como FALLADO.
3. analizar-resiliencia
   - Muestra impacto de fallar cada nodo y sugiere enlaces de redundancia si procede.

Ejemplo E: Probar optimización de ruta
1. optimizar-ruta H1 Serv1
   - Compara latencia actual vs. latencia con un enlace hipotético; recomienda si la mejora es significativa.

---

9. Buenas prácticas y consejos
------------------------------
- Use nombres únicos y descriptivos para los nodos (evite reutilizar nombres).
- Valide direcciones IP antes de agregarlas.
- Antes de probar `ping` o `traceroute`, ejecute `ver-grafo` para confirmar que las aristas están activas.
- Haga copias periódicas con `guardar` al trabajar en topologías importantes.
- Para análisis de resiliencia en redes grandes, tenga en cuenta que el algoritmo simula fallos uno por uno — puede tardar más en grafos grandes.
- Para visualización gráfica, asegúrese de que `python3` esté en PATH y el script de visualización instalado. El programa lanza el script como un proceso hijo.

---

10. Errores comunes y solución de problemas
------------------------------------------
- "IP inválida": revise formato IPv4; el programa exige a.b.c.d con cada octeto 0-255.
- "Nodo duplicado": el nombre ya existe, elija otro.
- "Dispositivo origen/destino no existe": compruebe con `ver-grafo` o `indice_por_nombre` implícito, corrija errores tipográficos.
- "No hay camino entre X y Y": o bien la topología no tiene rutas, o enlaces intermedios están fallados. Compruebe estados con `ver-grafo`.
- "No se pudo abrir el visualizador grafico": asegúrese de tener Python 3 y el script `src/verGrafoL.py`. Revise permisos de ejecución.
- "Fallo al guardar/cargar": compruebe permisos de archivo y rutas; el programa devolve error si no puede abrir el archivo.

---

11. Limitaciones y comportamiento esperado
-----------------------------------------
- Redes dirigidas: las aristas son dirigidas; si necesita comunicación bidireccional, debe crear aristas en ambos sentidos.
- Modelo de fiabilidad simple: la probabilidad de éxito por enlace es independiente y se multiplica; no hay modelado de retransmisiones ni colisiones.
- K-rutas aproximadas: la heurística que encuentra rutas alternativas no garantiza todas las K rutas óptimas (es una aproximación basada en desactivar aristas).
- El análisis de resiliencia se realiza con BFS simple y asume que la conectividad se evalúa en términos de número de nodos alcanzables desde un nodo de inicio activo.
- El sistema no implementa protocolos reales de enrutamiento; son simulaciones heurísticas basadas en pesos (latencia).
- No se implementa NAT, VLANs, ni detalles de capa 2/3 reales: el modelo es de alto nivel conceptual.

---

12. Preguntas frecuentes (FAQ)
------------------------------
P: ¿Puedo simular enlaces redundantes automáticamente?
R: El programa sugiere enlaces entre vecinos del nodo crítico en el análisis de resiliencia, pero no los agrega automáticamente. Debe usar `conectar-dispositivo` para crearlos.

P: ¿Cómo simulo un nodo caído (no un enlace)?
R: La función `fallar-disp` está comentada en el código (según la versión). Si su compilación tiene habilitado `fallar-disp`, úselo; si no, puede simular fallos de nodo marcando todas sus aristas como inactivas manualmente o extendiendo el código.

P: ¿Se guardan los estados (activo/fallado) al guardar?
R: Sí. Tanto el estado de nodos como el de aristas se persiste en el archivo de topología.

P: ¿Puedo cargar topologías grandes?
R: Sí, dentro de la memoria disponible y límites iniciales. El grafo crece dinámicamente. Para grafos muy grandes, operaciones como K-routes o analizar-resiliencia tomarán más tiempo.

---

13. Créditos y licencias
------------------------
- Este manual describe el funcionamiento del ejecutable provisto por el paquete de código que implementa un grafo de red, funciones de Dijkstra y utilidades de análisis de resiliencia y rutas.
- El uso de herramientas externas (p. ej. script Python para visualización) puede requerir software adicional con sus propias licencias.

---

Anexos: Ejemplo de sesión interactiva (simulada)
------------------------------------------------
net> nuevo-disp R1 10.0.0.1 router 200
[OK] Dispositivo agregado: R1

net> nuevo-disp H1 10.0.0.2 host 10
[OK] Dispositivo agregado: H1

net> conectar-dispositivo R1 H1 20 100 0.98
[OK] Conexion agregada: R1 -> H1

net> ver-grafo
Grafo: 2 vertices
 [0] R1 (10.0.0.1) - Tipo: ROUTER, Cap: 200, Estado: ACTIVO
    -> H1 (idx 1) | lat=20ms bw=100Mbps conf=0.98 estado=ACTIVO
 [1] H1 (10.0.0.2) - Tipo: HOST, Cap: 10, Estado: ACTIVO

net> ping R1 H1 3
[PING] Ruta seleccionada: R1 -> H1
[prueba 1] Respuesta de H1: tiempo=20.00 ms
[prueba 2] Respuesta de H1: tiempo=20.00 ms
[prueba 3] Respuesta de H1: tiempo=20.00 ms
ESTADISTICAS DE PING
3 paquetes transmitidos, 3 recibidos, 0.0% de pérdida
rtt min/prom/max = 20.00/20.00/20.00 ms

net> salir

---

Contacto y soporte
------------------
Para dudas sobre la lógica del simulador o reporte de errores, consulte la documentación del proyecto o contacte con el desarrollador/autores del repositorio donde obtuvo el código.

Fin del manual.