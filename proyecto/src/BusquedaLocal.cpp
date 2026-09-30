#include "BusquedaLocal.h"

#include <algorithm>
#include <numeric>
#include <optional>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

namespace {
struct Conexion {
    int origen, destino, costo;
};

// Kruskal usará el mismo orden de aristas para todos los candidatos.
// Ordenar una sola vez evita repetir ese trabajo miles de veces.
vector<Conexion> ordenarConexiones(const Grafo& grafo) {
    vector<Conexion> conexiones;
    for (int origen = 0; origen < grafo.cantidadNodos(); ++origen) {
        for (const Arista& arista : grafo.vecinos(origen)) {
            if (arista.costo < 0) throw invalid_argument("La búsqueda local necesita costos no negativos.");
            if (origen < arista.destino) conexiones.push_back({origen, arista.destino, arista.costo});
        }
    }
    sort(conexiones.begin(), conexiones.end(), [](const Conexion& a, const Conexion& b) {
        return tie(a.costo, a.origen, a.destino) < tie(b.costo, b.origen, b.destino);
    });
    return conexiones;
}

int representante(vector<int>& padre, int nodo) {
    while (padre[nodo] != nodo) {
        padre[nodo] = padre[padre[nodo]];
        nodo = padre[nodo];
    }
    return nodo;
}

optional<ResultadoSteiner> reconstruir(
    const Grafo& original, const vector<Conexion>& conexiones, const vector<bool>& activos) {
    const int nodos = original.cantidadNodos();
    // La clase Grafo numera desde 0. Compactamos los activos y guardamos
    // la equivalencia para recuperar sus identificadores después de la poda.
    vector<int> localAOriginal;
    vector<int> originalALocal(nodos, -1);
    for (int nodo = 0; nodo < nodos; ++nodo) {
        if (activos[nodo]) {
            originalALocal[nodo] = static_cast<int>(localAOriginal.size());
            localAOriginal.push_back(nodo);
        }
    }
    const int cantidad = static_cast<int>(localAOriginal.size());
    if (cantidad == 0) return nullopt;
    Grafo compacto(cantidad);
    for (int terminal : original.terminales()) {
        if (!activos[terminal]) return nullopt;
        compacto.agregarTerminal(originalALocal[terminal]);
    }

    vector<int> padre(nodos), tamanio(nodos, 1);
    iota(padre.begin(), padre.end(), 0);
    int elegidas = 0;
    for (const Conexion& conexion : conexiones) {
        if (elegidas == cantidad - 1) break;
        if (!activos[conexion.origen] || !activos[conexion.destino]) continue;
        int a = representante(padre, conexion.origen);
        int b = representante(padre, conexion.destino);
        if (a == b) continue;
        if (tamanio[a] < tamanio[b]) swap(a, b);
        padre[b] = a;
        tamanio[a] += tamanio[b];
        compacto.agregarArista(originalALocal[conexion.origen],
                              originalALocal[conexion.destino], conexion.costo);
        ++elegidas;
    }
    // Si quitar un nodo desconecta el subgrafo seleccionado, rechazar el cambio.
    if (elegidas != cantidad - 1) return nullopt;

    // Reutilizamos la poda y sus comprobaciones, sin cambiar Steiner.cpp.
    const auto podado = podarSteiner(compacto);
    ResultadoSteiner resultado{Grafo(nodos), vector<bool>(nodos, false),
        podado.costo, podado.cantidadNodos, podado.cantidadAristas};
    for (int terminal : original.terminales()) resultado.arbol.agregarTerminal(terminal);
    for (int local = 0; local < cantidad; ++local) {
        const int origen = localAOriginal[local];
        resultado.activos[origen] = podado.activos[local];
        for (const Arista& arista : podado.arbol.vecinos(local)) {
            if (local < arista.destino) {
                resultado.arbol.agregarArista(origen, localAOriginal[arista.destino], arista.costo);
            }
        }
    }
    return resultado;
}
}

ResultadoBusquedaLocal mejorarBusquedaLocal(
    const Grafo& original, const ResultadoSteiner& inicial, int maxPasadas) {
    if (maxPasadas < 0 || !validarSteiner(original, inicial)) {
        throw invalid_argument("Se necesita una solución inicial válida y un límite no negativo.");
    }
    const auto conexiones = ordenarConexiones(original);
    vector<bool> esTerminal(original.cantidadNodos(), false);
    for (int nodo : original.terminales()) esTerminal[nodo] = true;
    ResultadoBusquedaLocal resultado{inicial, 0, 0, 0, false, {inicial.costo}};

    auto evaluar = [&](const vector<bool>& activos) {
        ++resultado.candidatosEvaluados;
        auto candidata = reconstruir(original, conexiones, activos);
        if (!candidata) return false;
        if (!validarSteiner(original, *candidata)) {
            throw runtime_error("La reconstrucción produjo una solución inválida.");
        }
        if (candidata->costo >= resultado.solucion.costo) return false;
        resultado.solucion = move(*candidata);
        ++resultado.mejorasAceptadas;
        resultado.historialCostos.push_back(resultado.solucion.costo);
        return true;
    };

    // Antes de cambiar nodos, explorar todas las conexiones originales entre
    // los nodos actuales. El árbol de entrada puede no ser su mejor MST.
    evaluar(resultado.solucion.activos);

    while (maxPasadas == 0 || resultado.pasadas < maxPasadas) {
        bool mejoro = false;
        ++resultado.pasadas;
        for (int nodo = 0; nodo < original.cantidadNodos(); ++nodo) {
            if (esTerminal[nodo]) continue; // Nunca quitar un terminal.
            vector<bool> candidatos = resultado.solucion.activos;
            candidatos[nodo] = !candidatos[nodo]; // Inserción o eliminación.
            if (evaluar(candidatos)) mejoro = true;
        }
        if (!mejoro) {
            resultado.sinMejorasPendientes = true;
            break;
        }
    }
    return resultado;
}
