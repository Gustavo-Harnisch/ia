#include "Evolutivo.h"
#include "Heuristica.h"
#include "MST.h"
#include "Paralelismo.h"
#include "IntercambioCaminos.h"
#include "BusquedaLocal.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <tuple>

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

void probarIntercambioCaminos() {
    // Dos nodos nuevos son necesarios: agregar uno solo no sirve.
    Grafo g(6);
    g.agregarTerminal(0); g.agregarTerminal(1);
    g.agregarArista(0, 1, 10);
    g.agregarArista(0, 2, 1); g.agregarArista(2, 3, 1); g.agregarArista(3, 1, 1);
    g.agregarArista(0, 4, 1); g.agregarArista(4, 5, 1); g.agregarArista(5, 1, 1);
    ResultadoSteiner inicial{Grafo(6), {true, true, false, false, false, false}, 10, 2, 1};
    inicial.arbol.agregarTerminal(0); inicial.arbol.agregarTerminal(1);
    inicial.arbol.agregarArista(0, 1, 10);
    comprobar(mejorarBusquedaLocal(g, inicial).solucion.costo == 10, "La prueba no representa un mínimo local");
    const auto a = mejorarIntercambioCaminos(g, inicial, 0, 1);
    const auto b = mejorarIntercambioCaminos(g, inicial, 0, 4);
    comprobar(a.solucion.costo == 3 && validarSteiner(g, a.solucion), "No sustituye un camino con varios nodos nuevos");
    comprobar(a.solucion.activos == b.solucion.activos && a.historialCostos == b.historialCostos,
              "Los hilos alteran el intercambio");
    comprobar(a.sinMejorasPendientes && a.mejorasAceptadas == 1 && inicial.costo == 10,
              "Convergencia o entrada alterada");
    const auto limitada = mejorarIntercambioCaminos(g, inicial, 1, 2);
    comprobar(limitada.pasadas == 1 && !limitada.sinMejorasPendientes, "No respeta el límite de pasadas");

    // Un camino largo se elimina por completo; la reconexión puede terminar
    // en cualquier nodo de las componentes, preservando ramas con terminales.
    Grafo ramas(7);
    for (int t : {0, 1, 6}) ramas.agregarTerminal(t);
    for (const auto& e : vector<tuple<int,int,int>>{{0,2,1},{2,3,8},{3,1,8},{2,6,1},{0,4,1},{4,5,1},{5,1,1}})
        ramas.agregarArista(get<0>(e), get<1>(e), get<2>(e));
    ResultadoSteiner arbol{Grafo(7), {true,true,true,true,false,false,true}, 18, 5, 4};
    for (int t : ramas.terminales()) arbol.arbol.agregarTerminal(t);
    arbol.arbol.agregarArista(0,2,1); arbol.arbol.agregarArista(2,3,8);
    arbol.arbol.agregarArista(3,1,8); arbol.arbol.agregarArista(2,6,1);
    auto cambio = mejorarIntercambioCaminos(ramas, arbol, 0, 4);
    comprobar(cambio.solucion.costo == 5 && !cambio.solucion.activos[3]
              && validarSteiner(ramas, cambio.solucion), "Pierde ramas o no retira interiores antiguos");

    Grafo unico(1); unico.agregarTerminal(0);
    auto uno = mejorarIntercambioCaminos(unico, construirPorCaminos(unico, 0), 0, 4);
    comprobar(uno.solucion.costo == 0 && uno.caminosEvaluados == 0 && uno.sinMejorasPendientes,
              "Falla con un único terminal");
    Grafo ceros(3); ceros.agregarTerminal(0); ceros.agregarTerminal(2);
    ceros.agregarArista(0,1,0); ceros.agregarArista(1,2,0); ceros.agregarArista(0,2,0);
    auto cero = mejorarIntercambioCaminos(ceros, construirPorCaminos(ceros,0), 0, 2);
    comprobar(cero.solucion.costo == 0 && validarSteiner(ceros,cero.solucion), "Falla con costos cero");

    Grafo grande(3); grande.agregarTerminal(0); grande.agregarTerminal(2);
    grande.agregarArista(0,1,numeric_limits<int>::max());
    grande.agregarArista(1,2,numeric_limits<int>::max());
    grande.agregarArista(0,2,1);
    ResultadoSteiner largo{Grafo(3), {true,true,true}, 2LL*numeric_limits<int>::max(), 3, 2};
    largo.arbol.agregarTerminal(0); largo.arbol.agregarTerminal(2);
    largo.arbol.agregarArista(0,1,numeric_limits<int>::max());
    largo.arbol.agregarArista(1,2,numeric_limits<int>::max());
    comprobar(mejorarIntercambioCaminos(grande,largo,0,2).solucion.costo == 1, "Desbordamiento de costo de camino");
    for (int caso = 0; caso < 3; ++caso) {
        bool rechazado = false;
        auto entrada = inicial;
        if (caso == 1) ++entrada.costo;
        auto original = g;
        if (caso == 2) original.agregarArista(2,5,-1);
        try { mejorarIntercambioCaminos(original, entrada, caso == 0 ? -1 : 1, 2); }
        catch (const invalid_argument&) { rechazado = true; }
        comprobar(rechazado, "Intercambio acepta parámetros o costos inválidos");
    }
}

int main() {
    probarIntercambioCaminos();
    for (unsigned hilos : {1u, 4u}) {
        EjecutorParalelo ejecutor(hilos);
        vector<int> visitas(37, 0);
        for (int ronda = 0; ronda < 20; ++ronda) {
            ejecutor.ejecutar(visitas.size(), [&](size_t i) { ++visitas[i]; });
        }
        comprobar(all_of(visitas.begin(), visitas.end(), [](int n) { return n == 20; }), "Tareas perdidas o duplicadas");
        bool fallo = false;
        try {
            ejecutor.ejecutar(20, [](size_t i) { if (i == 3) throw runtime_error("Prueba"); });
        } catch (const runtime_error&) { fallo = true; }
        comprobar(fallo, "No propaga errores del trabajador");
        ejecutor.ejecutar(0, [](size_t) { throw runtime_error("Lote vacío"); });
        ejecutor.ejecutar(1, [&](size_t) { visitas[0] = 99; });
        comprobar(visitas[0] == 99, "No se recupera después de un error");
    }
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
    const auto secuencial = ejecutarEvolutivoElitista(denso, base, 60, 123, {}, 1);
    const auto paralelo = ejecutarEvolutivoElitista(denso, base, 60, 123, {}, 4);
    comprobar(secuencial.aristas == paralelo.aristas && secuencial.nodos == paralelo.nodos
              && secuencial.historialCostos == paralelo.historialCostos
              && secuencial.solucionesEvaluadas == paralelo.solucionesEvaluadas
              && secuencial.descendientesDiferentes == paralelo.descendientesDiferentes
              && secuencial.crucesRealizados == paralelo.crucesRealizados,
              "Cambiar hilos altera la evolución");
    for (int limite : {0, 1, 2}) {
        const auto a = mejorarPorCaminos(denso, limite, 1);
        const auto b = mejorarPorCaminos(denso, limite, 4);
        comprobar(a.solucion.costo == b.solucion.costo && a.terminalInicial == b.terminalInicial
                  && a.intentos == b.intentos && validarSteiner(denso, b.solucion),
                  "Heurística paralela no conserva resultados o límites");
    }
    Grafo desconectado(2);
    desconectado.agregarTerminal(0); desconectado.agregarTerminal(1);
    bool errorParalelo = false;
    try { mejorarPorCaminos(desconectado, 0, 2); }
    catch (const runtime_error&) { errorParalelo = true; }
    comprobar(errorParalelo, "Heurística paralela no propaga desconexión");
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
