# Contexto del proyecto: árbol de Steiner

## ¿Qué pide el profesor?

La actividad de la Clase 3 consiste en resolver una instancia del **problema del árbol de Steiner**. Hay que construir una red de costo bajo que conecte todos los nodos obligatorios (terminales). Se pueden usar nodos opcionales de Steiner si ayudan a reducir el costo.

La instancia descrita en el PDF tiene:

- 2.500 nodos en total.
- 417 nodos terminales, que deben quedar conectados.
- 2.083 nodos opcionales.
- 62.500 aristas con costos.

El archivo de instancia sigue el formato SteinLib y el PDF indica que se entrega junto con el enunciado. Se descargó `datos/e18.stp` del repositorio oficial: sus cantidades coinciden con las del PDF. La lectura de E18 ya está implementada; falta compararlo con el adjunto del profesor.

## ¿Esto es machine learning o deep learning?

Por lo que describe el enunciado, **no se pide entrenar un modelo de machine learning ni de deep learning**. No se habla de entrenar con ejemplos ni de usar redes neuronales. Es un problema de búsqueda y optimización combinatoria, un área que también forma parte de la inteligencia artificial.

El sistema debe encontrar una red que conecte los terminales y reducir su costo. Para eso el PDF solicita una solución base y al menos una técnica de mejora.

## Requisitos que aparecen en el PDF

1. Leer la instancia y modelarla como un grafo ponderado no dirigido.
2. Crear una solución base usando un árbol de expansión mínima (MST).
3. Implementar al menos una mejora, como una heurística constructiva o búsqueda local.
4. Podar nodos innecesarios, especialmente hojas que no sean terminales.
5. Validar con DFS o BFS que todos los terminales estén conectados y que la solución forme un árbol.
6. Calcular el costo, compararlo con el MST base y mostrar la reducción porcentual.
7. Preparar un reporte con el problema, la representación, los algoritmos, los resultados, las comparaciones y las dificultades.

El PDF propone como referencias de evaluación un MST de costo 2515, un MST podado de costo 892 y una solución óptima de costo 564. La calificación aumenta según la calidad de la solución factible.

## ¿Qué significa “algoritmo evolutivo elitista”?

Es una posible técnica de mejora; el PDF **no obliga a usarla**. “Elitista” significa que, en cada ronda, se conservan sin cambios algunas de las mejores soluciones encontradas.

Una versión conceptual sería:

1. Crear varias redes candidatas que conecten todos los terminales.
2. Calcular el costo de cada red. Menor costo significa mejor solución.
3. Elegir algunas redes buenas para generar variantes.
4. Modificar o combinar las redes, por ejemplo cambiando conexiones.
5. Revisar que cada variante siga conectando todos los terminales y podarla.
6. Copiar las mejores redes a la siguiente ronda sin modificarlas: eso es el elitismo.
7. Repetir el proceso y conservar la red factible de menor costo.

La parte difícil es generar cambios que mantengan una solución válida. Por eso conviene implementar primero el grafo, la validación, la solución base y la poda. Luego se puede agregar el algoritmo evolutivo como mejora.

## Ruta recomendada para aprender y construir el proyecto

Avanzar en pasos pequeños permite comprobar cada parte antes de agregar la siguiente:

1. **C++ básico:** variables, funciones, `struct`, `vector` y lectura de archivos con `ifstream`.
2. **Representación de grafos:** lista de adyacencia, nodos, aristas y costos.
3. **Recorridos:** BFS o DFS para comprobar conectividad.
4. **Lectura de la instancia:** interpretar el formato SteinLib y cargar nodos, terminales y aristas.
5. **Solución base:** implementar un MST y medir su costo. Una alternativa de partida es calcular caminos mínimos entre terminales, construir un MST con esas distancias y expandirlo en el grafo original.
6. **Poda y validación:** eliminar hojas que no sean terminales y volver a comprobar la conectividad.
7. **Mejora:** empezar con búsqueda local o, si el equipo lo elige, implementar una población evolutiva con elitismo.
8. **Evaluación y reporte:** comparar costos y explicar qué métodos se usaron.

## Organización actual del código

El proyecto combina módulos y una clase sencilla para representar el grafo:

```text
proyecto/
├── contexto.md
├── CMakeLists.txt
├── .gitignore
├── .vscode/settings.json
├── datos/
│   ├── README.md
│   ├── grafo.txt
│   └── e18.stp
├── salidas/                # Imágenes y descripción DOT generadas
└── src/
    ├── main.cpp
    ├── Grafo.h
    ├── Grafo.cpp
    ├── Lectura.h
    ├── Lectura.cpp
    ├── Grafica.h
    ├── Grafica.cpp
    ├── MST.h
    ├── MST.cpp
    ├── Steiner.h
    ├── Steiner.cpp
    ├── Heuristica.h
    ├── Heuristica.cpp
    ├── BusquedaLocal.h
    ├── BusquedaLocal.cpp
    ├── Evolutivo.h
    └── Evolutivo.cpp
```

- `main.cpp`: punto de entrada. Coordina la lectura, la comprobación de conectividad y la generación de imágenes. Los datos del ejemplo están fuera del código.
- `Grafo.h`: declara la clase `Grafo`, la estructura `Arista` y las funciones de recorrido y conectividad. Es la descripción de las operaciones disponibles.
- `Grafo.cpp`: implementa esas operaciones. Guarda las aristas en ambos sentidos y recorre el grafo con BFS.
- `Lectura.h` y `Lectura.cpp`: detectan y validan tanto el TXT del ejemplo como SteinLib STP 1.0 no dirigido para construir un `Grafo`. En STP convierten los identificadores de base 1 a base 0.
- `Grafica.h` y `Grafica.cpp`: describen el grafo en formato DOT e invocan Graphviz para dibujarlo en PNG y SVG.
- `MST.h` y `MST.cpp`: construyen el MST con Kruskal, devuelven su costo y comprueban que sea un árbol conexo.
- `Steiner.h` y `Steiner.cpp`: podan hojas no terminales del MST y verifican la solución resultante.
- `Heuristica.h` y `Heuristica.cpp`: construyen árboles agregando caminos mínimos hacia terminales pendientes y comparan distintos inicios.
- `BusquedaLocal.h` y `BusquedaLocal.cpp`: reconstruyen el árbol sobre los nodos seleccionados y prueban insertar o eliminar un nodo opcional, conservando solo mejoras válidas.
- `Evolutivo.h` y `Evolutivo.cpp`: generan descendientes mediante mutación y cruce, conservan la élite y evitan duplicados.
- `CMakeLists.txt`: enumera los archivos que deben compilarse juntos, establece C++17 y localiza Graphviz (comando `dot`).
- `datos/grafo.txt`: contiene nodos, terminales y conexiones; su formato está explicado en `datos/README.md`. Es un formato de aprendizaje, distinto de SteinLib.
- `salidas/`: contiene `grafo.dot`, `grafo.png` y `grafo.svg`. Se regeneran al ejecutar; los cambios de datos deben hacerse en el TXT.
- `.vscode/settings.json`: adapta el botón Run Code a la compilación de este proyecto con varios archivos. También hay una configuración en la carpeta del curso para poder abrir cualquiera de las dos carpetas en VS Code.

Los datos del grafo son privados: se accede a ellos mediante operaciones como `agregarArista`, `agregarTerminal`, `vecinos` y `terminales`. Las funciones `recorrer` y `terminalesConectados` trabajan con esa interfaz pública.

La lectura de SteinLib, el MST base, la poda, la heurística de caminos mínimos y la búsqueda local ya funcionan. El MST conecta todos los nodos; la poda conserva los necesarios dentro de ese árbol. La heurística construye otro árbol a partir del grafo original. Se valida la conectividad, la estructura de árbol, los terminales, la pertenencia de las aristas al grafo original y el costo. El algoritmo evolutivo elitista está implementado y se describe más abajo.

## MST base implementado

El algoritmo está separado en `src/MST.cpp`. Kruskal ordena las conexiones por costo y agrega una conexión cuando une dos grupos distintos, evitando formar ciclos. Termina con N - 1 conexiones. Si no puede conectar todos los nodos, informa que no existe un MST del grafo completo.

Resultados comprobados:

| Entrada | Nodos | Aristas del MST | Costo |
| --- | ---: | ---: | ---: |
| `grafo.txt` | 6 | 5 | 12 |
| `e18.stp` | 2500 | 2499 | 2515 |

El costo de E18 coincide con la referencia de MST base del PDF (categoría de 4 puntos, sujeta a la evaluación del profesor). Para el ejemplo pequeño se generan tanto `salidas/grafo.png` como `salidas/grafo_mst.png`, para comparar la red original y el árbol.

## Poda de Steiner implementada

`Steiner.cpp` elimina repetidamente hojas que no son terminales. Una cola permite procesar las nuevas hojas que aparecen después de cada eliminación. El MST original se conserva para comparar sus resultados.

Para conservar los identificadores, el resultado mantiene un vector `activos`: los nodos eliminados quedan sin aristas y marcados como inactivos. No forman parte de la solución. Por eso su validación comprueba los nodos activos, en vez de exigir que estén conectados los 2.500 nodos originales.

Resultado verificado para E18:

- Costo del MST: **2515**.
- Costo después de podar: **947**.
- Nodos conservados: **946**, incluidos los **417 terminales**.
- Nodos eliminados: **1554**.
- Aristas conservadas: **945**.
- Reducción del costo: **62,35 %**.
- Solución válida y sin hojas opcionales pendientes de poda.

La referencia de MST podado del PDF es **892**, que aún no se alcanza. Cuando hay empates, distintos MST de igual costo pueden dar costos diferentes después de la poda. No se garantiza un puntaje adicional solo por ejecutar este paso; el costo y la calidad serán evaluados por el profesor. La poda produce una solución factible de Steiner, pero no garantiza la óptima.

Las imágenes actuales siguen mostrando el grafo original y el MST; todavía no se dibuja el árbol podado con sus nodos activos.

## Heurística de caminos mínimos

El módulo independiente `Heuristica.cpp` empieza en un terminal. Dijkstra encuentra el terminal pendiente más cercano al árbol actual y se incorpora su camino completo. Los nuevos nodos se convierten en orígenes de distancia cero para actualizar las distancias sin reiniciar todo el cálculo. El proceso termina cuando todos los terminales están conectados.

Cada camino se une al árbol una sola vez y termina en un terminal. Por construcción no quedan hojas opcionales para podar. Todos los resultados se verifican con `validarSteiner`. Esta es una heurística constructiva con múltiples inicios; aún no es un algoritmo evolutivo ni garantiza el óptimo.

Se puede llamar este módulo sin ejecutar antes el MST:

```cpp
Grafo grafo = leerGrafo(archivo);
ResultadoHeuristica resultado = mejorarPorCaminos(grafo);
// Alternativa más rápida: probar solo los primeros 20 terminales.
// ResultadoHeuristica resultado = mejorarPorCaminos(grafo, 20);
cout << resultado.solucion.costo << '\n';
```

Resultados para E18 al probar los **417 terminales** de inicio:

| Método | Costo |
| --- | ---: |
| MST | 2515 |
| MST podado | 947 |
| Mejor heurística de caminos mínimos | **628** |

La mejor solución tiene **571 nodos y 570 aristas**, conserva los **417 terminales** y reduce el costo del MST podado en **33,69 %**. El mejor inicio fue el nodo interno **1732**, que corresponde al **1733** del archivo SteinLib. La ejecución medida tomó aproximadamente **9,27 segundos**, incluyendo la validación de cada candidato; el tiempo varía según el equipo y la compilación.

El costo 628 está dentro del rango 601–891 de la pauta (6–7 puntos), sujeto a evaluación del profesor. El óptimo de referencia, 564, todavía no se alcanza. El programa mantiene la comparación con la poda y muestra el menor costo entre ambos métodos, porque la heurística no garantiza mejorar cualquier instancia.

Las imágenes automáticas continúan mostrando el ejemplo original y su MST; aún no representan los nodos activos de la solución heurística.

## Búsqueda local implementada

El módulo `BusquedaLocal.cpp` recibe cualquier `ResultadoSteiner` válido. Primero reconstruye el MST con todas las conexiones originales entre sus nodos activos. Luego prueba activar o desactivar un nodo opcional, reconstruye y poda. Nunca elimina terminales; rechaza selecciones desconectadas y comprueba cada candidato reconstruido con `validarSteiner`.

Se aceptan únicamente costos estrictamente menores. El proceso hace pasadas hasta completar una sin mejoras o alcanzar el límite indicado (20 por defecto; 0 significa sin límite de pasadas). La ausencia de mejoras se refiere solo a los cambios de un nodo y la reconstrucción determinista explorados, no demuestra optimalidad global.

```cpp
ResultadoBusquedaLocal local = mejorarBusquedaLocal(grafo, heuristica.solucion);
cout << local.solucion.costo << '\n';
```

Resultado medido sobre E18:

| Etapa | Costo |
| --- | ---: |
| MST | 2515 |
| MST podado | 947 |
| Heurística de caminos mínimos | 628 |
| Búsqueda local | **597** |

Se aceptaron **17 mejoras**, se evaluaron **4167 candidatos** y se completaron **2 pasadas**. La búsqueda local tomó **31,84 segundos** en la ejecución medida, además del tiempo de construcción inicial. Primero la reconstrucción bajó de 628 a 613; luego los cambios de nodos redujeron el costo hasta 597. La segunda pasada terminó sin mejoras. Los tiempos dependen del equipo y la compilación.

597 corresponde al rango 581–600 de la pauta (8–10 puntos), sujeto a evaluación del profesor. Quedan 33 unidades hasta el óptimo de referencia 564. Otros experimentos posibles son aplicar búsqueda local a distintas soluciones iniciales e intercambiar caminos completos. La recombinación de soluciones élite ya se utiliza en el evolutivo.

Se comprobaron también ejemplos de inserción y eliminación beneficiosas, rechazos por desconexión, un único terminal, entradas inválidas y el límite de pasadas. El MST, la poda y la heurística originales siguen disponibles como módulos independientes.

## Evolutivo elitista implementado

El código anterior copiaba el padre en `mutarSolucion`: aumentar generaciones repetía exactamente los mismos árboles. La versión actual implementa:

- **Mutación:** agrega de uno a cuatro nodos opcionales con al menos dos vecinos activos, reconstruye el MST con desempates aleatorios y poda hojas opcionales. Así puede cambiar conexiones y retirar nodos que dejan de ser necesarios.
- **Cruce:** une los nodos activos de dos padres diferentes y reconstruye/poda un árbol sobre el subgrafo inducido. Los hijos solo utilizan conexiones y costos del grafo original.
- **Selección:** torneo para elegir padres; sobreviven las 14 mejores soluciones distintas y hasta dos alternativas aleatorias. Las firmas incluyen las aristas y sus costos para eliminar clones, sin confundir árboles diferentes de igual costo.
- **Diversidad adicional:** cada diez generaciones se construye una solución desde otro terminal de inicio.
- **Validación y elitismo:** cada descendiente se valida antes de entrar a la población; se conserva el mejor costo y se devuelve su árbol completo. El historial permite comprobar que el mejor costo nunca aumenta.
- **Paralelismo reproducible:** hasta cuatro tareas simultáneas, cada una con su propio generador aleatorio. La semilla predeterminada es 42.

Prueba con `e18.stp`, 100 generaciones y semilla 42: **597 → 589**, solución válida, 1610 candidatos evaluados, 1600 descendientes diferentes de sus padres, 792 cruces y población final de 16 árboles distintos. El evolutivo tardó aproximadamente 1,80 segundos, sin contar las etapas anteriores. Con **1000 generaciones y semilla 42**, el costo bajó a **583**, con solución válida, 16100 candidatos evaluados, 16000 descendientes diferentes, 7992 cruces y aproximadamente 16,12 segundos de evolutivo. La última mejora ocurrió en la generación 247. No se garantiza mejorar en cada generación ni alcanzar el óptimo de referencia 564; puede haber períodos sin mejora.

El monitor muestra generaciones completadas, evaluaciones, descendientes diferentes, tamaño de población sin duplicados y mejor costo. Estos datos distinguen una búsqueda estancada temporalmente de la repetición de clones del código anterior.

Las pruebas de regresión comprueban una mutación que debe bajar de costo 10 a 2, validez del árbol retornado, elitismo, repetibilidad, límite de generaciones, cruces, costos de 64 bits, un terminal, nodos aislados, empates y rechazo de parámetros inválidos. Se ejecutan con sanitizadores de memoria y comportamiento indefinido:

```bash
bash tests/ejecutar.sh
```

## Primer ejercicio de aprendizaje

1. Para este ejercicio, cambiar `archivoPredeterminado` a `"grafo.txt"` en `src/main.cpp`. Abrir `datos/grafo.txt`: la línea `0 1 4` conecta los nodos 0 y 1 con costo 4.
2. Consultar `src/Lectura.cpp` para ver cómo esos números se convierten en llamadas a `grafo.agregarArista`.
3. Eliminar la línea `4 5 2` y cambiar `ARISTAS 6` por `ARISTAS 5`. Ejecutar desde `src/main.cpp`: el terminal 5 quedará desconectado y aparecerá aislado en la imagen. Restaurar ambos cambios después del ejercicio.
4. Cambiar costos: la conectividad no cambia, porque BFS solo comprueba si existen caminos, pero las etiquetas del dibujo sí cambian. Los costos se utilizarán al implementar la optimización.

## Ejecutar desde VS Code

Abrir la carpeta `proyecto` (o la carpeta del curso), abrir `src/main.cpp` y pulsar **▶ Run Code**. Se guardan los archivos y `ejecutar.sh` compila todos los módulos con C++17 en una carpeta temporal local. Esto permite ejecutar incluso cuando el proyecto está abierto mediante un montaje SFTP/GVFS. La configuración local es necesaria porque este programa tiene varios `.cpp`; la configuración global sigue sirviendo para archivos independientes en otras carpetas.

Al ejecutar el ejemplo pequeño, abrir `salidas/grafo.png` en VS Code para ver el dibujo. Los nodos azules son terminales, los grises son opcionales y los números de las conexiones son sus costos. También se genera `grafo.svg`, que se puede ampliar sin perder calidad. Graphviz ya está instalado en este PC; será necesario instalarlo si el proyecto se lleva a otro equipo.

Por defecto se carga `datos/e18.stp` y se muestran 2.500 nodos, 62.500 aristas, 417 terminales y su conectividad. Para volver al dibujo pequeño, cambiar `archivoPredeterminado` a `"grafo.txt"` en `src/main.cpp`. Las imágenes solo se generan para grafos de hasta 80 nodos y 200 aristas; el grafo completo de E18 se lee y recorre, pero no se dibuja automáticamente.

CMake genera una carpeta `build/` con el ejecutable y archivos de compilación. No hay que editar su contenido. Al agregar un nuevo `.cpp`, incluirlo en la lista `add_executable` de `CMakeLists.txt`.

Como alternativa, desde la carpeta `proyecto`:

```bash
bash ejecutar.sh datos/e18.stp 100 42
# Para probar el ejemplo pequeño:
bash ejecutar.sh datos/grafo.txt 20 42
```

Se requieren `g++` y Graphviz (`dot`). El script elimina su carpeta temporal al terminar. La entrada predeterminada se resuelve desde la ubicación del proyecto, independientemente de la carpeta de la terminal.

Los argumentos opcionales del ejecutable son `archivo generaciones semilla`. Se conservó el límite de **1.000.000 de generaciones** que estaba configurado en `Evolutivo.h`; Run Code sin argumentos usa ese límite y puede tardar bastante. Para una prueba breve, usar el comando de 100 generaciones anterior. El tercer y cuarto argumento de `ejecutarEvolutivoElitista` también permiten elegir límite y semilla desde C++.

CMake también está disponible al trabajar en un sistema de archivos local o directamente en el servidor. Si se trasladó el proyecto, usar una carpeta de compilación nueva para evitar cachés con rutas anteriores:

```bash
cmake -S . -B build-nuevo -DCMAKE_BUILD_TYPE=Release
cmake --build build-nuevo
./build-nuevo/proyecto
```

## Organización del equipo

Si son cuatro integrantes, una distribución posible es:

- Lectura de SteinLib y representación del grafo.
- Solución base MST y cálculo del costo.
- Poda, validación y técnica de mejora.
- Integración, experimentos y reporte.

Con tres integrantes, se pueden combinar la integración y el reporte con otra tarea.

## Enlaces y datos pendientes

- Repositorio indicado en el PDF: [SteinLib, conjunto E](https://steinlib.zib.de/showset.php?E).
- Se descargó `datos/e18.stp` del [conjunto E oficial](https://steinlib.zib.de/download/E.tgz) y se verificaron 2.500 nodos, 62.500 aristas y 417 terminales. Coincide con la descripción del ejercicio; falta compararlo con el adjunto del profesor. El lector admite ambos formatos y el programa carga E18 por defecto.


## Imágenes de la mejor solución durante la ejecución

Desde esta versión, el programa dibuja la solución inicial de la búsqueda local y actualiza las imágenes cada vez que el evolutivo encuentra un costo estrictamente menor. No es necesario esperar a que termine el millón de generaciones.

Cada ejecución crea su propia carpeta `salidas/evolutivo_<identificador>_semilla_<semilla>/`, cuya ruta se imprime en la terminal:

- `mejor.png`: imagen de tamaño acotado para ver o compartir.
- `mejor.svg`: imagen vectorial para ampliar y leer los identificadores.
- `mejor.dot`: descripción exacta de los nodos activos, aristas y costos dibujados.
- `mejoras.csv`: generación, costo, cantidad de nodos y aristas de cada mejora guardada.

Azul identifica terminales obligatorios y naranja identifica nodos opcionales utilizados. Los nodos descartados no aparecen. Las etiquetas de las aristas muestran costos originales y los identificadores de nodos son internos (base 0; en SteinLib corresponden al identificador del archivo menos uno). El título indica costo, generación y semilla.

Graphviz calcula las posiciones para visualizar el árbol: la optimización elige conexiones y nodos, no coordenadas geográficas. Se generan archivos nuevos temporales y se reemplaza cada imagen al terminar de dibujar, evitando abrir un PNG o SVG incompleto. El renderizado ocurre solo en las mejoras y añade tiempo a la ejecución; no modifica la selección ni su aleatoriedad. Si falla el guardado de imágenes, se informa y la búsqueda continúa.

Para verlo mientras corre en el servidor, abrir la carpeta `salidas` desde SFTP o VS Code Remote SSH, entrar a la carpeta indicada en la terminal y abrir `mejor.png`. Volver a abrir o recargar la vista después de una actualización; la actualización automática depende del visor. Usar `mejor.svg` para ampliar árboles grandes.

Una ejecución que ya estaba corriendo con un binario anterior no incorpora esta función. Hace falta iniciar una nueva ejecución con el código actualizado; el programa no recupera la solución que el proceso anterior conserva solamente en memoria.
