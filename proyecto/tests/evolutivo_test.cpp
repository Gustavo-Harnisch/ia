#include "Evolutivo.h"
#include "Heuristica.h"
#include "MST.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace std;

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw runtime_error(mensaje);
}

void comprobarResultado(const Grafo& original, const ResultadoEvolutivo& resultado,
                        long long inicial, int generaciones) {
    ResultadoSteiner arbol{Grafo(original.cantidadNodos()),
        vector<bool>(original.cantidadNodos(), false), 0,
        static_cast<int>(resultado.nodos.size()), static_cast<int>(resultado.aristas.size())};
    for (int t : original.terminales()) arbol.arbol.agregarTerminal(t);
    for (int n : resultado.nodos) arbol.activos[n] = true;
    for (const auto& [a, b] : resultado.aristas) {
        bool existe = false;
        for (const auto& arista : original.vecinos(a)) {
            if (arista.destino == b) {
                arbol.arbol.agregarArista(a, b, arista.costo);
                arbol.costo += arista.costo;
                existe = true;
                break;
            }
        }
        comprobar(existe, "Arista inventada");
    }
    comprobar(validarSteiner(original, arbol), "Árbol devuelto inválido");
    comprobar(arbol.costo == resultado.costo && resultado.valida, "Costo o validación incorrectos");
    comprobar(resultado.costo <= inicial, "Se perdió el elitismo");
    comprobar(resultado.generaciones == generaciones, "No respeta el límite");
    comprobar(resultado.historialCostos.size() == static_cast<size_t>(generaciones + 1), "Historial incompleto");
    comprobar(is_sorted(resultado.historialCostos.rbegin(), resultado.historialCostos.rend()), "El mejor costo aumentó");
    comprobar(resultado.poblacionFinal > 0 && resultado.poblacionFinal <= 16, "Población fuera de límite");
}

int main() {
    // Regresión del error original: la mutación debe sustituir una conexión cara.
    Grafo grafo(4);
    grafo.agregarTerminal(0); grafo.agregarTerminal(1);
    grafo.agregarArista(0, 1, 10);
    grafo.agregarArista(0, 2, 1); grafo.agregarArista(2, 1, 1);
    // Nodo 3 aislado: nunca debe activarse ni desconectar la solución.
    ResultadoSteiner inicial{Grafo(4), {true, true, false, false}, 10, 2, 1};
    inicial.arbol.agregarTerminal(0); inicial.arbol.agregarTerminal(1);
    inicial.arbol.agregarArista(0, 1, 10);
    const auto mutada = mutarSolucion(grafo, inicial, 42);
    comprobar(validarSteiner(grafo, mutada) && mutada.costo == 2, "La mutación sigue copiando el padre");
    comprobar(inicial.costo == 10 && !inicial.activos[2], "Mutó la entrada");
    vector<pair<int, long long>> avisos;
    const auto resultado = ejecutarEvolutivoElitista(grafo, inicial, 5, 42,
        [&](const ResultadoSteiner& mejor, int generacion) {
            comprobar(validarSteiner(grafo, mejor), "El observador recibió una solución inválida");
            avisos.emplace_back(generacion, mejor.costo);
        });
    comprobar(avisos.size() == 2 && avisos.front() == make_pair(0, 10LL)
              && avisos.back() == make_pair(1, 2LL), "Avisos de mejora incorrectos");
    comprobarResultado(grafo, resultado, 10, 5);
    comprobar(resultado.costo == 2 && resultado.descendientesDiferentes > 0, "No explora alternativas");
    comprobar(resultado.crucesRealizados > 0, "No hay cruce");
    const auto repetida = ejecutarEvolutivoElitista(grafo, inicial, 5, 42);
    comprobar(resultado.aristas == repetida.aristas && resultado.historialCostos == repetida.historialCostos,
              "La misma semilla no reproduce la ejecución");

    // Un único terminal y un árbol sin alternativas son óptimos sin mutaciones.
    Grafo unico(1); unico.agregarTerminal(0);
    const auto uno = construirPorCaminos(unico, 0);
    comprobarResultado(unico, ejecutarEvolutivoElitista(unico, uno, 2, 7), 0, 2);
    Grafo cadena(3); cadena.agregarTerminal(0); cadena.agregarTerminal(2);
    cadena.agregarArista(0, 1, numeric_limits<int>::max());
    cadena.agregarArista(1, 2, numeric_limits<int>::max());
    const auto largo = podarSteiner(construirMST(cadena).arbol);
    comprobarResultado(cadena, ejecutarEvolutivoElitista(cadena, largo, 2, 7), largo.costo, 2);
    comprobar(largo.costo > numeric_limits<int>::max(), "No se probó costo de 64 bits");

    // Empates, ramas opcionales y semillas diferentes: comprobar invariantes.
    Grafo denso(8);
    for (int t : {0, 3, 7}) denso.agregarTerminal(t);
    for (int a = 0; a < 8; ++a) {
        for (int b = a + 1; b < 8; ++b) denso.agregarArista(a, b, (a + b) % 3);
    }
    const auto base = construirPorCaminos(denso, 0);
    for (unsigned semilla = 0; semilla < 20; ++semilla) {
        comprobar(validarSteiner(denso, mutarSolucion(denso, base, semilla)), "Mutación inválida con empates");
    }
    comprobarResultado(denso, ejecutarEvolutivoElitista(denso, base, 8, 123), base.costo, 8);
    for (int limite : {0, -1}) {
        bool rechazado = false;
        try { ejecutarEvolutivoElitista(grafo, inicial, limite); }
        catch (const invalid_argument&) { rechazado = true; }
        comprobar(rechazado, "Aceptó límite inválido");
    }
    auto invalida = inicial; ++invalida.costo;
    bool rechazado = false;
    try { ejecutarEvolutivoElitista(grafo, invalida, 1); }
    catch (const invalid_argument&) { rechazado = true; }
    comprobar(rechazado, "Aceptó solución inicial inválida");
    cout << "Pruebas del evolutivo: OK\n";
}
