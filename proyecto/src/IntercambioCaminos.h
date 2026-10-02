#ifndef INTERCAMBIO_CAMINOS_H
#define INTERCAMBIO_CAMINOS_H

#include "Steiner.h"
#include <vector>

struct ResultadoIntercambioCaminos {
    ResultadoSteiner solucion;
    int caminosEvaluados = 0;
    int mejorasAceptadas = 0;
    int pasadas = 0;
    bool sinMejorasPendientes = false;
    std::vector<long long> historialCostos;
};

// Retira caminos cuyos interiores son opcionales de grado 2 y reconecta las
// dos componentes por un camino mínimo en el grafo original. Acepta la mejor
// mejora estricta de cada pasada, con desempate independiente de los hilos.
// maxPasadas: 0 hasta estabilizarse; numeroHilos: 0 automático, 1 secuencial.
ResultadoIntercambioCaminos mejorarIntercambioCaminos(
    const Grafo& original, const ResultadoSteiner& inicial,
    int maxPasadas = 10, unsigned numeroHilos = 0);

#endif
