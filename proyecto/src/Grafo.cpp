#include "Grafo.h"

#include <algorithm>
#include <queue>
#include <stdexcept>

using namespace std;

Grafo::Grafo(int cantidadNodos) {
    if (cantidadNodos < 0) {
        throw invalid_argument("La cantidad de nodos no puede ser negativa.");
    }
    adyacencia_.resize(cantidadNodos);
}

int Grafo::cantidadNodos() const {
    return static_cast<int>(adyacencia_.size());
}

void Grafo::validarNodo(int nodo) const {
    if (nodo < 0 || nodo >= cantidadNodos()) {
        throw out_of_range("El nodo no existe en el grafo.");
    }
}

void Grafo::agregarArista(int origen, int destino, int costo) {
    validarNodo(origen);
    validarNodo(destino);

    // Grafo no dirigido: la conexión se registra en ambos sentidos.
    adyacencia_[origen].push_back({destino, costo});
    adyacencia_[destino].push_back({origen, costo});
    conexiones_.push_back({origen, destino, costo});
}

void Grafo::agregarTerminal(int nodo) {
    validarNodo(nodo);
    // Evitar que un mismo terminal se registre dos veces.
    if (find(terminales_.begin(), terminales_.end(), nodo) == terminales_.end()) {
        terminales_.push_back(nodo);
    }
}

const vector<Arista>& Grafo::vecinos(int nodo) const {
    validarNodo(nodo);
    return adyacencia_[nodo];
}

const vector<int>& Grafo::terminales() const {
    return terminales_;
}

const vector<ConexionGrafo>& Grafo::conexiones() const {
    return conexiones_;
}

vector<bool> recorrer(const Grafo& grafo, int inicio) {
    // También comprueba que el nodo inicial exista, incluso si el grafo está vacío.
    grafo.vecinos(inicio);
    vector<bool> visitado(grafo.cantidadNodos(), false);
    queue<int> pendientes;

    visitado[inicio] = true;
    pendientes.push(inicio);

    // BFS: procesar primero los nodos que se descubrieron antes.
    while (!pendientes.empty()) {
        int actual = pendientes.front();
        pendientes.pop();

        for (const Arista& arista : grafo.vecinos(actual)) {
            if (!visitado[arista.destino]) {
                visitado[arista.destino] = true;
                pendientes.push(arista.destino);
            }
        }
    }
    return visitado;
}

bool terminalesConectados(const Grafo& grafo) {
    // Sin terminales no hay ninguna conexión obligatoria que comprobar.
    if (grafo.terminales().empty()) {
        return true;
    }

    const auto alcanzable = recorrer(grafo, grafo.terminales().front());
    for (int terminal : grafo.terminales()) {
        if (!alcanzable[terminal]) {
            return false;
        }
    }
    return true;
}
