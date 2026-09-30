#ifndef BUSQUEDA_LOCAL_H
#define BUSQUEDA_LOCAL_H

#include "Steiner.h"
#include <vector>

struct ResultadoBusquedaLocal {
    ResultadoSteiner solucion;
    int candidatosEvaluados;
    int mejorasAceptadas;
    int pasadas;
    bool sinMejorasPendientes;
    std::vector<long long> historialCostos;
};

// Reorganiza las conexiones y prueba agregar/quitar un nodo opcional por vez.
// Acepta únicamente soluciones válidas de menor costo. No altera la entrada.
// El límite evita ejecuciones indefinidas; 0 permite continuar hasta estabilizarse.
ResultadoBusquedaLocal mejorarBusquedaLocal(
    const Grafo& original, const ResultadoSteiner& inicial, int maxPasadas = 20);

#endif
