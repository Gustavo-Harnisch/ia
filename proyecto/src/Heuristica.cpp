#include "Heuristica.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

ResultadoSteiner construirPorCaminos(const Grafo& grafo, int terminalInicial) {
    const int nodos = grafo.cantidadNodos();
    vector<bool> terminal(nodos, false);
    for (int nodo : grafo.terminales()) terminal[nodo] = true;
    if (terminalInicial < 0 || terminalInicial >= nodos || !terminal[terminalInicial]) {
        throw invalid_argument("El inicio de la heurística debe ser un terminal del grafo.");
    }
    for (int nodo = 0; nodo < nodos; ++nodo) {
        for (const Arista& arista : grafo.vecinos(nodo)) {
            if (arista.costo < 0) throw invalid_argument("Dijkstra necesita costos no negativos.");
        }
    }

    ResultadoSteiner resultado{Grafo(nodos), vector<bool>(nodos, false), 0, 1, 0};
    for (int nodo : grafo.terminales()) resultado.arbol.agregarTerminal(nodo);
    resultado.activos[terminalInicial] = true;
    int pendientes = static_cast<int>(grafo.terminales().size()) - 1;

    // Dijkstra usa distancias desde el árbol actual, no solo desde el inicio.
    // Cada nodo añadido al árbol se convierte en un nuevo origen de distancia 0.
    const long long infinito = numeric_limits<long long>::max();
    vector<long long> distancia(nodos, infinito);
    vector<int> padre(nodos, -1), costoPadre(nodos, 0);
    using Entrada = pair<long long, int>; // Distancia, identificador del nodo.
    priority_queue<Entrada, vector<Entrada>, greater<Entrada>> cola;
    distancia[terminalInicial] = 0;
    cola.push({0, terminalInicial});

    while (pendientes > 0 && !cola.empty()) {
        const auto [distanciaActual, actual] = cola.top();
        cola.pop();
        if (distanciaActual != distancia[actual]) continue; // Entrada antigua.

        if (terminal[actual] && !resultado.activos[actual]) {
            // Es el terminal pendiente más cercano: reconstruir su camino
            // hasta tocar el árbol. Así cada camino se une una sola vez y no crea ciclos.
            int nodo = actual;
            while (!resultado.activos[nodo]) {
                const int anterior = padre[nodo];
                if (anterior < 0) throw runtime_error("No se pudo reconstruir el camino.");
                resultado.arbol.agregarArista(nodo, anterior, costoPadre[nodo]);
                resultado.costo += costoPadre[nodo];
                ++resultado.cantidadAristas;
                ++resultado.cantidadNodos;
                resultado.activos[nodo] = true;
                if (terminal[nodo]) --pendientes;

                // Propagar solo las mejoras que introduce este nuevo origen.
                distancia[nodo] = 0;
                cola.push({0, nodo});
                nodo = anterior;
            }
            continue;
        }

        for (const Arista& arista : grafo.vecinos(actual)) {
            const long long nueva = distanciaActual + arista.costo;
            if (nueva < distancia[arista.destino]) {
                distancia[arista.destino] = nueva;
                padre[arista.destino] = actual;
                costoPadre[arista.destino] = arista.costo;
                cola.push({nueva, arista.destino});
            }
        }
    }
    if (pendientes != 0) throw runtime_error("Hay terminales que no se pueden conectar.");

    // Cada rama termina en un terminal, por lo que no deja hojas opcionales que podar.
    return resultado;
}

ResultadoHeuristica mejorarPorCaminos(const Grafo& grafo, int maxInicios) {
    if (grafo.terminales().empty() || maxInicios < 0) {
        throw invalid_argument("Se necesita al menos un terminal y un límite no negativo.");
    }
    const int total = static_cast<int>(grafo.terminales().size());
    const int intentos = maxInicios == 0 ? total : min(total, maxInicios);
    optional<ResultadoHeuristica> mejor;

    for (int i = 0; i < intentos; ++i) {
        const int inicio = grafo.terminales()[i];
        ResultadoSteiner candidata = construirPorCaminos(grafo, inicio);
        if (!validarSteiner(grafo, candidata)) {
            throw runtime_error("La heurística generó una solución inválida.");
        }
        if (!mejor || candidata.costo < mejor->solucion.costo) {
            mejor = ResultadoHeuristica{move(candidata), inicio, intentos};
        }
    }
    return move(*mejor);
}
