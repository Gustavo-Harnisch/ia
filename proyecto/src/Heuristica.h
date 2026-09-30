#ifndef HEURISTICA_H
#define HEURISTICA_H

#include "Steiner.h"

struct ResultadoHeuristica {
    ResultadoSteiner solucion;
    int terminalInicial; // Identificador interno, desde 0.
    int intentos;
};

// Conecta sucesivamente el terminal pendiente más cercano al árbol.
ResultadoSteiner construirPorCaminos(const Grafo& grafo, int terminalInicial);

// Prueba distintos terminales de inicio y conserva el menor costo.
// maxInicios = 0 significa probar todos; un valor positivo limita los intentos.
ResultadoHeuristica mejorarPorCaminos(const Grafo& grafo, int maxInicios = 0);

#endif
