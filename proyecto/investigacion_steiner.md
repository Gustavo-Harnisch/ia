# Investigación: cómo acercarse al costo 564 de E18

Fecha de consulta: 27 de septiembre de 2026.

## Situación del proyecto

Resultados medidos por nuestro código: MST 2515, MST podado 947 y heurística de caminos mínimos con 417 inicios 628. El PDF asigna 15 puntos al costo óptimo 564. Faltan reducir 64 unidades para igualarlo.

Esta investigación revisó publicaciones, documentación y archivos de repositorios. No se ejecutaron solvers externos ni se obtuvo una nueva solución de costo 564 durante esta etapa. En el momento de esta investigación la mejor solución propia era 628. Actualización posterior: la búsqueda local propia alcanzó 597; ver `contexto.md`.

## Evidencia de que 564 es alcanzable

La tabla I, página 6, de [Solving the Steiner Tree Problem in graphs with Variable Neighborhood Descent](https://people.cs.kuleuven.be/~danny.weyns/STCS/STCS-16_paper_1.pdf) identifica E18 con 2500 nodos, 62500 aristas y 417 terminales. Reporta 597 para su VND y 564 para el algoritmo de Polzin, con un tiempo histórico de 21,6 segundos para este último. Es una comparación publicada, no una medición en nuestro equipo. Tampoco implica que implementar cualquier búsqueda local produzca 564.

## Búsqueda local especializada

[Fast local search for the Steiner problem in graphs, Uchoa y Werneck, 2012](https://doi.org/10.1145/2133803.2184448) estudia inserción y eliminación de nodos de Steiner, intercambio de caminos clave y eliminación de vértices clave. El trabajo presenta búsquedas eficientes; una implementación sencilla nuestra puede ser más lenta.

La idea útil es modificar también qué nodos opcionales participan, además de cambiar conexiones. Un óptimo local solo garantiza que no se encontró una mejora dentro de los cambios explorados.

## Evolución con soluciones élite

[A Robust and Scalable Algorithm for the Steiner Problem in Graphs, Pajor, Uchoa y Werneck, 2014](https://arxiv.org/abs/1412.2787) combina múltiples construcciones, perturbación de costos, búsqueda local y recombinación de soluciones élite. Las secciones 2.1–2.4 explican esos componentes. Es la referencia que más se acerca a la idea de evolución elitista discutida en este proyecto.

El repositorio [ms-steiner-puw](https://github.com/Luftschlange/ms-steiner-puw) declara implementar ese trabajo. Se consultó su lista de archivos y se leyeron los módulos de inserción y élite. Archivos útiles para estudiar:

- [LSVertexInsertion.h](https://github.com/Luftschlange/ms-steiner-puw/blob/master/src/LSVertexInsertion.h): inserción de nodos opcionales y evaluación de mejoras.
- [LSKeyPath.h](https://github.com/Luftschlange/ms-steiner-puw/blob/master/src/LSKeyPath.h): referencia para intercambio de caminos.
- [elite.h](https://github.com/Luftschlange/ms-steiner-puw/blob/master/src/elite.h): conjunto de soluciones élite.
- [perturbation.h](https://github.com/Luftschlange/ms-steiner-puw/blob/master/src/perturbation.h): referencia para diversificación.

No se confirmó mediante ejecución que esta copia del repositorio alcance 564 en nuestro archivo E18.

## Método exacto como referencia

[SCIP-Jack](https://scipjack.zib.de/) es un solver de Steiner que lee STP y permite exportar soluciones. Su documentación describe un enfoque con reducciones, heurísticas y branch-and-cut, que permite demostrar optimalidad cuando termina con cotas coincidentes. Véase la [descripción del método](https://www.scipopt.org/doc-6.0.1/html/STP_PROBLEM.php).

La web oficial indica que la versión 2.2 se obtiene solicitándola a su autor. El [repositorio público antiguo](https://github.com/dRehfeldt/SCIPJack-Steiner-tree-solver) advierte que necesita fuentes de SCIP y SoPlex y recomienda una versión más reciente. No es una dependencia que ya esté integrada en nuestro programa.

## Propuesta de implementación para nuestro proyecto

Esta es una adaptación propuesta, no una reproducción ya verificada de los resultados de los papers:

1. Crear `BusquedaLocal.h/.cpp`. Recalcular un MST usando todas las conexiones originales entre los nodos activos, podarlo y evaluar inserciones o eliminaciones de nodos opcionales. Si una eliminación desconecta terminales, rechazarla. Conservar identificadores mediante un mapa al trabajar con subgrafos.
2. Incorporar intercambios de caminos para explorar cambios que una sola inserción no permite.
3. Aplicar búsqueda local a varias de las soluciones iniciales. Guardar soluciones distintas, no solo la más barata.
4. Crear `Evolutivo.h/.cpp`: recombinar soluciones, introducir variaciones y conservar las mejores con diversidad. Registrar semilla y tiempo. Las perturbaciones guían la búsqueda; la evaluación final siempre utiliza los costos originales.
5. Exportar las aristas de la mejor solución y verificar de nuevo conectividad, terminales, ausencia de ciclos y suma del costo. Para demostrar optimalidad de forma independiente, comparar también con cotas de un método exacto.

Hitos de costo de la pauta: 600 o menos; luego 580 o menos; finalmente 564. Son objetivos, no resultados garantizados. Obtener y validar 564 cumple el objetivo de costo máximo de la pauta; la entrega también debe incluir los algoritmos y el reporte solicitados.
