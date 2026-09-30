#include "MST.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <vector>

using namespace std;

namespace {
struct Conexion {
    int origen;
    int destino;
    int costo;
};

// Disjoint Set Union (DSU): registra qué nodos ya están conectados.
// Permite evitar ciclos sin recorrer todo el árbol por cada conexión.
class Conjuntos {
public:
    explicit Conjuntos(int cantidad) : padre(cantidad), tamanio(cantidad, 1) {
        iota(padre.begin(), padre.end(), 0); // Al inicio cada nodo es su propio grupo.
    }

    int representante(int nodo) {
        if (padre[nodo] != nodo) {
            padre[nodo] = representante(padre[nodo]);
        }
        return padre[nodo];
    }

    bool unir(int a, int b) {
        a = representante(a);
        b = representante(b);
        if (a == b) return false; // Ya conectados: esta arista crearía un ciclo.

        // Incorporar el grupo más pequeño en el más grande.
        if (tamanio[a] < tamanio[b]) swap(a, b);
        padre[b] = a;
        tamanio[a] += tamanio[b];
        return true;
    }

private:
    vector<int> padre;
    vector<int> tamanio;
};
}

ResultadoMST construirMST(const Grafo& grafo) {
    const int nodos = grafo.cantidadNodos();
    if (nodos == 0) throw invalid_argument("El MST necesita al menos un nodo.");

    // 1. Reunir cada conexión una sola vez, sin duplicar sus dos sentidos.
    vector<Conexion> conexiones;
    for (int origen = 0; origen < nodos; ++origen) {
        for (const Arista& arista : grafo.vecinos(origen)) {
            if (origen < arista.destino) {
                conexiones.push_back({origen, arista.destino, arista.costo});
            }
        }
    }

    // 2. Ordenar por costo. Los identificadores resuelven empates de forma estable.
    sort(conexiones.begin(), conexiones.end(), [](const Conexion& a, const Conexion& b) {
        return tie(a.costo, a.origen, a.destino) < tie(b.costo, b.origen, b.destino);
    });

    ResultadoMST resultado{Grafo(nodos), 0, 0};
    for (int terminal : grafo.terminales()) {
        resultado.arbol.agregarTerminal(terminal);
    }
    Conjuntos grupos(nodos);

    // 3. Aceptar conexiones que unan grupos diferentes hasta completar N - 1.
    for (const Conexion& conexion : conexiones) {
        if (grupos.unir(conexion.origen, conexion.destino)) {
            resultado.arbol.agregarArista(conexion.origen, conexion.destino, conexion.costo);
            resultado.costo += conexion.costo;
            ++resultado.cantidadAristas;
            if (resultado.cantidadAristas == nodos - 1) break;
        }
    }

    if (resultado.cantidadAristas != nodos - 1) {
        throw runtime_error("No existe un MST de todos los nodos: el grafo está desconectado.");
    }
    return resultado;
}

bool esArbol(const Grafo& grafo) {
    const int nodos = grafo.cantidadNodos();
    if (nodos == 0) return false;

    size_t conexionesDirigidas = 0;
    for (int nodo = 0; nodo < nodos; ++nodo) {
        conexionesDirigidas += grafo.vecinos(nodo).size();
    }
    if (conexionesDirigidas != 2 * static_cast<size_t>(nodos - 1)) return false;

    // Verificación independiente de Kruskal, usando el BFS que ya implementamos.
    const auto visitado = recorrer(grafo, 0);
    return all_of(visitado.begin(), visitado.end(), [](bool valor) { return valor; });
}
