#include "IntercambioCaminos.h"
#include "Paralelismo.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <stdexcept>
#include <utility>

using namespace std;

namespace {
struct Conexion { int a, b, costo; };
struct Camino {
    int inicio, fin;
    long long costo = 0;
    vector<int> aristas, interiores;
};
struct ArbolIndexado {
    vector<Conexion> aristas;
    vector<vector<pair<int, int>>> vecinos; // Nodo vecino e índice de arista.
    vector<Camino> caminos;
};

ArbolIndexado indexar(const ResultadoSteiner& solucion) {
    const int n = solucion.arbol.cantidadNodos();
    ArbolIndexado indice{{}, vector<vector<pair<int, int>>>(n), {}};
    vector<bool> clave(n, false);
    for (int t : solucion.arbol.terminales()) clave[t] = true;
    for (int a = 0; a < n; ++a) {
        if (solucion.activos[a] && solucion.arbol.vecinos(a).size() != 2) clave[a] = true;
        for (const auto& e : solucion.arbol.vecinos(a)) {
            if (a >= e.destino) continue;
            const int id = static_cast<int>(indice.aristas.size());
            indice.aristas.push_back({a, e.destino, e.costo});
            indice.vecinos[a].emplace_back(e.destino, id);
            indice.vecinos[e.destino].emplace_back(a, id);
        }
    }
    for (int a = 0; a < n; ++a) {
        if (!clave[a]) continue;
        for (const auto& [vecino, id] : indice.vecinos[a]) {
            Camino camino{a, vecino, indice.aristas[id].costo, {id}, {}};
            int anterior = a, actual = vecino;
            while (!clave[actual]) {
                camino.interiores.push_back(actual);
                const auto siguiente = indice.vecinos[actual][0].first == anterior
                    ? indice.vecinos[actual][1] : indice.vecinos[actual][0];
                camino.aristas.push_back(siguiente.second);
                camino.costo += indice.aristas[siguiente.second].costo;
                anterior = actual;
                actual = siguiente.first;
            }
            camino.fin = actual;
            if (a < actual) indice.caminos.push_back(move(camino));
        }
    }
    return indice;
}

optional<ResultadoSteiner> intercambiar(const Grafo& original,
    const ResultadoSteiner& actual, const ArbolIndexado& indice, const Camino& camino) {
    if (camino.costo == 0) return nullopt;
    const int n = original.cantidadNodos();
    vector<bool> retirada(indice.aristas.size(), false), activos = actual.activos;
    for (int id : camino.aristas) retirada[id] = true;
    for (int nodo : camino.interiores) activos[nodo] = false;

    // Marcar una componente; los demás nodos activos forman la otra.
    vector<bool> componente(n, false);
    vector<int> pendientes{camino.inicio};
    componente[camino.inicio] = true;
    for (size_t i = 0; i < pendientes.size(); ++i) {
        for (const auto& [vecino, id] : indice.vecinos[pendientes[i]]) {
            if (!retirada[id] && !componente[vecino]) {
                componente[vecino] = true;
                pendientes.push_back(vecino);
            }
        }
    }
    using Entrada = pair<long long, int>;
    priority_queue<Entrada, vector<Entrada>, greater<Entrada>> cola;
    vector<long long> distancia(n, numeric_limits<long long>::max());
    vector<int> padre(n, -1), costoPadre(n, 0);
    for (int nodo : pendientes) {
        distancia[nodo] = 0;
        cola.emplace(0, nodo);
    }
    int destino = -1;
    while (!cola.empty()) {
        const auto [costo, nodo] = cola.top(); cola.pop();
        if (costo != distancia[nodo]) continue;
        // Con costos no negativos ninguna ruta pendiente podrá mejorar.
        if (costo >= camino.costo) break;
        if (activos[nodo] && !componente[nodo]) { destino = nodo; break; }
        for (const auto& e : original.vecinos(nodo)) {
            const long long nuevo = costo + e.costo;
            if (nuevo < camino.costo && nuevo < distancia[e.destino]) {
                distancia[e.destino] = nuevo;
                padre[e.destino] = nodo;
                costoPadre[e.destino] = e.costo;
                cola.emplace(nuevo, e.destino);
            }
        }
    }
    if (destino == -1) return nullopt;
    vector<Conexion> conexiones;
    for (size_t id = 0; id < indice.aristas.size(); ++id) {
        if (!retirada[id]) conexiones.push_back(indice.aristas[id]);
    }
    for (int nodo = destino; padre[nodo] != -1; nodo = padre[nodo]) {
        conexiones.push_back({padre[nodo], nodo, costoPadre[nodo]});
        activos[nodo] = activos[padre[nodo]] = true;
    }
    // Compactar permite reutilizar la poda de árboles conexos con nodos inactivos.
    vector<int> local(n, -1), global;
    for (int nodo = 0; nodo < n; ++nodo) {
        if (activos[nodo]) { local[nodo] = static_cast<int>(global.size()); global.push_back(nodo); }
    }
    Grafo compacto(static_cast<int>(global.size()));
    for (int t : original.terminales()) compacto.agregarTerminal(local[t]);
    for (const auto& e : conexiones) compacto.agregarArista(local[e.a], local[e.b], e.costo);
    const auto podado = podarSteiner(compacto);
    ResultadoSteiner resultado{Grafo(n), vector<bool>(n, false), podado.costo,
                              podado.cantidadNodos, podado.cantidadAristas};
    for (int t : original.terminales()) resultado.arbol.agregarTerminal(t);
    for (size_t i = 0; i < global.size(); ++i) {
        resultado.activos[global[i]] = podado.activos[i];
        for (const auto& e : podado.arbol.vecinos(static_cast<int>(i))) {
            if (static_cast<int>(i) < e.destino)
                resultado.arbol.agregarArista(global[i], global[e.destino], e.costo);
        }
    }
    if (!validarSteiner(original, resultado)) throw runtime_error("Intercambio de caminos inválido.");
    return resultado;
}
}

ResultadoIntercambioCaminos mejorarIntercambioCaminos(
    const Grafo& original, const ResultadoSteiner& inicial, int maxPasadas, unsigned numeroHilos) {
    if (maxPasadas < 0 || !validarSteiner(original, inicial))
        throw invalid_argument("El intercambio necesita una solución válida y un límite no negativo.");
    for (int nodo = 0; nodo < original.cantidadNodos(); ++nodo)
        for (const auto& e : original.vecinos(nodo))
            if (e.costo < 0) throw invalid_argument("El intercambio necesita costos no negativos.");
    ResultadoIntercambioCaminos resultado{inicial, 0, 0, 0, false, {inicial.costo}};
    const unsigned hilos = resolverHilos(numeroHilos, max(1, inicial.cantidadAristas));
    EjecutorParalelo ejecutor(hilos);
    while (maxPasadas == 0 || resultado.pasadas < maxPasadas) {
        const auto indice = indexar(resultado.solucion);
        optional<ResultadoSteiner> mejor;
        ++resultado.pasadas;
        // Resultados acotados a un lote; nunca se muta el árbol compartido.
        for (size_t lote = 0; lote < indice.caminos.size(); lote += hilos) {
            const size_t cantidad = min<size_t>(hilos, indice.caminos.size() - lote);
            vector<optional<ResultadoSteiner>> candidatas(cantidad);
            ejecutor.ejecutar(cantidad, [&](size_t i) {
                candidatas[i] = intercambiar(original, resultado.solucion, indice, indice.caminos[lote + i]);
            });
            resultado.caminosEvaluados += static_cast<int>(cantidad);
            for (auto& candidata : candidatas) {
                if (candidata && candidata->costo < resultado.solucion.costo
                    && (!mejor || candidata->costo < mejor->costo)) mejor = move(candidata);
            }
        }
        if (!mejor) { resultado.sinMejorasPendientes = true; break; }
        resultado.solucion = move(*mejor);
        ++resultado.mejorasAceptadas;
        resultado.historialCostos.push_back(resultado.solucion.costo);
    }
    return resultado;
}
