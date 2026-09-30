#ifndef STEINER_H
#define STEINER_H

#include "Grafo.h"
#include <vector>

struct ResultadoSteiner {
    // Se conservan los identificadores originales. Los nodos podados quedan
    // sin aristas y con activos[nodo] = false; no pertenecen a la solución.
    Grafo arbol;
    std::vector<bool> activos;
    long long costo;
    int cantidadNodos;
    int cantidadAristas;
};

// Poda repetidamente las hojas no terminales de un árbol conexo.
// Produce una solución factible, no garantiza el óptimo de Steiner.
ResultadoSteiner podarSteiner(const Grafo& arbol);

// Verifica el árbol de nodos activos, terminales, aristas originales y costo.
bool validarSteiner(const Grafo& original, const ResultadoSteiner& solucion);

#endif
