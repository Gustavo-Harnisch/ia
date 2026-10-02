#ifndef GRAFO_H
#define GRAFO_H

#include <vector>

// Una conexión hacia otro nodo, con su costo.
struct Arista {
    int destino;
    int costo;
};

struct ConexionGrafo {
    int origen;
    int destino;
    int costo;
};

// El .h declara qué operaciones ofrece la clase; el .cpp las implementa.
class Grafo {
public:
    explicit Grafo(int cantidadNodos);

    void agregarArista(int origen, int destino, int costo);
    void agregarTerminal(int nodo);

    int cantidadNodos() const;
    const std::vector<Arista>& vecinos(int nodo) const;
    const std::vector<int>& terminales() const;
    // Una entrada por conexión, en el orden en que se leyó/agregó.
    const std::vector<ConexionGrafo>& conexiones() const;

private:
    // Cada posición guarda las conexiones de un nodo.
    std::vector<std::vector<Arista>> adyacencia_;
    std::vector<int> terminales_;
    std::vector<ConexionGrafo> conexiones_;

    void validarNodo(int nodo) const;
};

// Por ahora estas funciones acompañan al grafo. Más adelante se pueden
// trasladar a un módulo de algoritmos junto con el MST y la poda.
std::vector<bool> recorrer(const Grafo& grafo, int inicio);
bool terminalesConectados(const Grafo& grafo);

#endif
