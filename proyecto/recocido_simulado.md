# Recocido simulado sobre nodos de Steiner

`src/RecocidoSimuladoSteiner.h/.cpp` porta los algoritmos de `principal.py` y
`secundario.py`. Se integran en `main.cpp` mediante dos modos de ejecución:

```bash
# Desde proyecto/. Equivalente a principal.py: 600 segundos de recocido.
bash ejecutar.sh --recocido-tiempo datos/e18.stp 600

# Equivalente a secundario.py: 600 segundos y semilla 1.
bash ejecutar.sh --recocido-bloques datos/e18.stp 600 1

# Prueba corta; cero segundos ejecuta solamente los 30 inicios.
bash ejecutar.sh --recocido-tiempo datos/e18.stp 5 1
```

Después del selector, los argumentos opcionales son `archivo segundos semilla`.
Por defecto se usa E18 y 600 segundos. Por tiempo se genera una semilla al azar
y se imprime; por bloques la semilla predeterminada es 1, como en el script.
La ejecución habitual `archivo generaciones semilla hilos` continúa disponible
para el evolutivo. Cada modo de recocido realiza su propia inicialización.

Ambos conservan los 30 terminales iniciales elegidos al azar con reemplazo,
la incorporación del terminal más cercano mediante Dijkstra incremental,
Kruskal sobre los nodos activos y la poda repetida de hojas no terminales.
La ordenación de aristas por costo es estable y respeta el orden del archivo;
`Grafo::conexiones()` permite conservarlo. El MST y MST podado informados por
estos modos usan ese mismo orden de desempate de los scripts.

Los movimientos mantienen sus probabilidades: 30 % intercambio de un Steiner
por un vecino, 35 % eliminación de un Steiner y 35 % adición de un vecino.
El candidato se elige primero sorteando un nodo activo y después uno de sus
vecinos. Se rechazan subgrafos desconectados. Se aceptan costos iguales o
menores, y aumentos con probabilidad `exp(-diferencia / temperatura)`.
El estado aceptado contiene los nodos que sobreviven a la poda. La mejor
solución se actualiza únicamente ante una mejora estricta.

| Regla | Por tiempo (`principal.py`) | Por bloques (`secundario.py`) |
| --- | --- | --- |
| Temperatura inicial/final | 0.3 / 0.08 | 0.3 / 0.07 |
| Enfriamiento | Geométrico por tiempo transcurrido | Geométrico dentro de 200000 intentos |
| Intervalo del bloque | No aplica | `avance1` a `min(1, avance1 + 0.03)` |
| Reinicio de semilla | No | `semilla * 1000 + bloque` |
| Sin nodos Steiner activos | Omite todos los movimientos | Permite agregar |
| Control del tiempo | Antes de cada intento | Antes de cada bloque completo |

El tiempo se mide desde el inicio del recocido, después de la construcción.
La variante por bloques puede sobrepasar el límite al completar su último
bloque, igual que el original. Los avisos por tiempo aparecen cada 10000
intentos cuando se llega al punto de impresión del script; por bloques se
informa al completar cada bloque.

La traducción conserva las reglas y parámetros del algoritmo. No reproduce
bit a bit una ejecución Python/Numba: utiliza `std::mt19937` y distribuciones
de C++, y enumera los conjuntos de nodos por identificador ascendente, mientras
Python utiliza el orden de sus `set`. Esto puede cambiar elecciones y
desempates de terminales equidistantes. Además, al detenerse por tiempo, el
número de intentos depende de la velocidad del equipo. La misma semilla no
garantiza el mismo árbol ni costo entre lenguajes o ejecuciones por tiempo.

La representación interna sigue usando identificadores desde cero y costos
acumulados de 64 bits. Para distancias se usa el máximo de 64 bits en lugar
del centinela `10**9`. Se validan entradas inválidas y se admite un único
terminal/nodo sin acceder a vecinos inexistentes. Estas protecciones cubren
casos donde los scripts originales fallan; no incorporan otra heurística.

Para usarlo desde C++:

```cpp
#include "RecocidoSimuladoSteiner.h"
auto resultado = ejecutarRecocidoSimuladoSteiner(
    grafo, VarianteRecocido::PorBloques, 600, 1);
// resultado.solucion contiene el árbol y resultado.valida su verificación.
```

Validación:

```bash
bash tests/ejecutar.sh
cmake -S . -B /tmp/steiner-build -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/steiner-build -j 4
python3 tests/comparar_recocido_python.py /tmp/steiner-build/proyecto
```

Las pruebas con sanitizadores comprueban los desempates de Kruskal, la
construcción, el límite temporal, los bloques completos, ambas reglas ante
ausencia de Steiner, conservación de la mejor solución, costos de 64 bits,
árboles válidos y rechazo de entradas inválidas. La comparación Python extrae
las funciones originales de ambos archivos y contrasta MST podado y costos
constructivos sobre 40 grafos pequeños con empates y conexiones paralelas;
no ejecuta ni necesita NumPy/Numba y no compara trayectorias aleatorias.
