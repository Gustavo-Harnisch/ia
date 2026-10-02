#include "RecocidoSimuladoSteiner.h"

#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

using namespace std;

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw runtime_error(mensaje);
}

int main() {
    // Los empates de Kruskal deben respetar la entrada, incluso con extremos
    // invertidos. Cambiar el desempate cambia el costo después de podar.
    Grafo empates(4);
    empates.agregarTerminal(0); empates.agregarTerminal(3);
    empates.agregarArista(2, 1, 1);
    empates.agregarArista(1, 0, 1);
    empates.agregarArista(3, 1, 1);
    empates.agregarArista(0, 3, 1);
    for (auto variante : {VarianteRecocido::PorTiempo, VarianteRecocido::PorBloques}) {
        const auto r = ejecutarRecocidoSimuladoSteiner(empates, variante, 0, 1);
        comprobar(r.costoMST == 3 && r.costoMSTPodado == 2, "Se alteró el orden de Kruskal/poda");
        comprobar(r.costoInicial == 1 && r.solucion.costo == 1 && r.iteraciones == 0,
                  "Construcción por caminos o límite cero incorrecto");
        comprobar(validarSteiner(empates, r.solucion), "Solución con empates inválida");
    }

    // Todos los inicios eligen dos conexiones directas de costo 5: costo 10.
    // El hub opcional cuesta 3+3+3=9, pero cada ruta por él cuesta 6 > 5.
    // Sin Steiner activos principal no se mueve; secundario sí puede agregarlo.
    Grafo hub(4);
    for (int t : {0, 1, 2}) hub.agregarTerminal(t);
    hub.agregarArista(0, 1, 5); hub.agregarArista(1, 2, 5); hub.agregarArista(0, 2, 5);
    for (int t : {0, 1, 2}) hub.agregarArista(t, 3, 3);
    const auto tiempo = ejecutarRecocidoSimuladoSteiner(hub, VarianteRecocido::PorTiempo, 0.005, 1);
    comprobar(tiempo.costoInicial == 10 && tiempo.solucion.costo == 10 && tiempo.iteraciones > 0,
              "La variante principal cambió su regla sin nodos Steiner");
    int avisos = 0;
    const auto bloques = ejecutarRecocidoSimuladoSteiner(hub, VarianteRecocido::PorBloques, 0.005, 1,
        [&](const ProgresoRecocido& p) {
            ++avisos;
            comprobar(p.iteraciones == p.bloques * 200000 && p.mejorCosto == 9,
                      "Bloque incompleto o no permite agregar sin nodos Steiner");
        });
    comprobar(bloques.costoInicial == 10 && bloques.solucion.costo == 9 && avisos > 0,
              "La variante secundaria no encuentra el hub");
    comprobar(bloques.iteraciones == bloques.bloques * 200000 && validarSteiner(hub, bloques.solucion),
              "Iteraciones del bloque o árbol incorrectos");
    comprobar(hub.conexiones().size() == 6, "Se modificó el grafo original");

    // Validez con ciclos, paralelas, ceros, hojas y candidatos desconectados.
    mt19937 azar(23);
    for (int caso = 0; caso < 10; ++caso) {
        Grafo g(12);
        for (int t : {0, 5, 11}) g.agregarTerminal(t);
        for (int i = 1; i < 12; ++i) g.agregarArista(i - 1, i, 1 + azar() % 10);
        for (int i = 0; i < 20; ++i) {
            const int a = azar() % 12, b = azar() % 12;
            if (a != b) g.agregarArista(a, b, azar() % 10);
        }
        const auto a = ejecutarRecocidoSimuladoSteiner(g, VarianteRecocido::PorTiempo, 0, 37);
        const auto b = ejecutarRecocidoSimuladoSteiner(g, VarianteRecocido::PorTiempo, 0, 37);
        comprobar(a.solucion.activos == b.solucion.activos && a.costoInicial == b.costoInicial,
                  "La construcción no es reproducible con semilla fija");
        const auto r = ejecutarRecocidoSimuladoSteiner(g, VarianteRecocido::PorTiempo, 0.002, 37);
        comprobar(validarSteiner(g, r.solucion) && r.solucion.costo <= r.costoInicial,
                  "Se perdió la mejor solución válida");
    }
    for (auto variante : {VarianteRecocido::PorTiempo, VarianteRecocido::PorBloques}) {
        Grafo unico(1); unico.agregarTerminal(0);
        const auto r = ejecutarRecocidoSimuladoSteiner(unico, variante, 0.001, 1);
        comprobar(r.solucion.costo == 0 && validarSteiner(unico, r.solucion), "Falla con un nodo");
        Grafo grande(3); grande.agregarTerminal(0); grande.agregarTerminal(2);
        grande.agregarArista(0, 1, numeric_limits<int>::max());
        grande.agregarArista(1, 2, numeric_limits<int>::max());
        const auto largo = ejecutarRecocidoSimuladoSteiner(grande, variante, 0, 1);
        comprobar(largo.solucion.costo == 2LL * numeric_limits<int>::max(), "Costo de 64 bits incorrecto");
    }
    for (int caso = 0; caso < 5; ++caso) {
        Grafo g(2);
        if (caso != 0) g.agregarTerminal(0);
        if (caso != 1) g.agregarArista(0, 1, caso == 2 ? -1 : 1);
        const double limite = caso == 3 ? -1 : caso == 4 ? numeric_limits<double>::infinity() : 0;
        bool rechazo = false;
        try { ejecutarRecocidoSimuladoSteiner(g, VarianteRecocido::PorTiempo, limite); }
        catch (const invalid_argument&) { rechazo = true; }
        comprobar(rechazo, "No rechaza entrada inválida");
    }
    cout << "Pruebas de recocido simulado Steiner correctas.\n";
}
