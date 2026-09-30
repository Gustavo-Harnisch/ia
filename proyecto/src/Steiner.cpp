#include "Steiner.h"
#include "MST.h"

#include <queue>
#include <stdexcept>
#include <vector>

using namespace std;

ResultadoSteiner podarSteiner(const Grafo& arbol) {
    if (!esArbol(arbol)) {
        throw invalid_argument("La poda necesita un árbol conexo; ejecuta primero el MST.");
    }
    if (arbol.terminales().empty()) {
        throw invalid_argument("La poda de Steiner necesita al menos un terminal.");
    }

    const int nodos = arbol.cantidadNodos();
    vector<bool> esTerminal(nodos, false);
    vector<bool> activo(nodos, true);
    vector<int> grado(nodos);
    queue<int> hojas;

    for (int terminal : arbol.terminales()) esTerminal[terminal] = true;
    for (int nodo = 0; nodo < nodos; ++nodo) {
        grado[nodo] = static_cast<int>(arbol.vecinos(nodo).size());
        if (!esTerminal[nodo] && grado[nodo] <= 1) hojas.push(nodo);
    }

    // Al quitar una hoja puede aparecer otra: repetir hasta que no queden.
    while (!hojas.empty()) {
        const int hoja = hojas.front();
        hojas.pop();
        if (!activo[hoja]) continue;

        activo[hoja] = false;
        for (const Arista& arista : arbol.vecinos(hoja)) {
            const int vecino = arista.destino;
            if (activo[vecino]) {
                --grado[vecino];
                if (!esTerminal[vecino] && grado[vecino] <= 1) hojas.push(vecino);
            }
        }
        grado[hoja] = 0;
    }

    // Reconstruir únicamente las conexiones conservadas, sin modificar el MST.
    ResultadoSteiner resultado{Grafo(nodos), activo, 0, 0, 0};
    for (int terminal : arbol.terminales()) resultado.arbol.agregarTerminal(terminal);
    for (int origen = 0; origen < nodos; ++origen) {
        if (!activo[origen]) continue;
        ++resultado.cantidadNodos;
        for (const Arista& arista : arbol.vecinos(origen)) {
            if (activo[arista.destino] && origen < arista.destino) {
                resultado.arbol.agregarArista(origen, arista.destino, arista.costo);
                resultado.costo += arista.costo;
                ++resultado.cantidadAristas;
            }
        }
    }
    return resultado;
}

bool validarSteiner(const Grafo& original, const ResultadoSteiner& solucion) {
    const int nodos = original.cantidadNodos();
    if (original.terminales().empty() || solucion.arbol.cantidadNodos() != nodos
        || solucion.activos.size() != static_cast<size_t>(nodos)
        || solucion.arbol.terminales() != original.terminales()) return false;

    for (int terminal : original.terminales()) {
        if (!solucion.activos[terminal]) return false;
    }

    // BFS desde un terminal: todos los nodos seleccionados deben ser alcanzables.
    const auto alcanzable = recorrer(solucion.arbol, original.terminales().front());
    int nodosActivos = 0, aristas = 0;
    long long costo = 0;
    for (int origen = 0; origen < nodos; ++origen) {
        if (!solucion.activos[origen]) {
            if (!solucion.arbol.vecinos(origen).empty()) return false;
            continue;
        }
        ++nodosActivos;
        if (!alcanzable[origen]) return false;

        for (const Arista& arista : solucion.arbol.vecinos(origen)) {
            if (!solucion.activos[arista.destino] || origen == arista.destino) return false;
            if (origen > arista.destino) continue;

            bool existe = false;
            for (const Arista& candidata : original.vecinos(origen)) {
                if (candidata.destino == arista.destino && candidata.costo == arista.costo) {
                    existe = true;
                    break;
                }
            }
            if (!existe) return false;
            ++aristas;
            costo += arista.costo;
        }
    }
    // Conectividad + N activos - 1 aristas garantiza que no hay ciclos.
    return aristas == nodosActivos - 1
        && nodosActivos == solucion.cantidadNodos
        && aristas == solucion.cantidadAristas
        && costo == solucion.costo;
}
