#ifndef MST_H
#define MST_H

#include "Grafo.h"

// El resultado conserva todos los nodos y terminales del grafo original.
struct ResultadoMST {
    Grafo arbol;
    long long costo;
    int cantidadAristas;
};

// Kruskal: elegir las conexiones más baratas sin formar ciclos.
// Si el grafo no es conexo, no existe un MST que abarque todos sus nodos.
ResultadoMST construirMST(const Grafo& grafo);

// Comprueba conectividad de TODOS los nodos y exactamente N - 1 aristas.
// Esa combinación garantiza un árbol para este grafo no dirigido.
bool esArbol(const Grafo& grafo);

#endif
