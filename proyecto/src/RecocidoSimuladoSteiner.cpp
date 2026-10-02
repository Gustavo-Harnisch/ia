#include "RecocidoSimuladoSteiner.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>

using namespace std;

namespace {
// Equivalente a evaluar() de secundario.py. Los arreglos se reutilizan;
// también evalúa principal.py, cuyo Kruskal y poda producen el mismo árbol.
class Evaluador {
public:
    const Grafo& grafo;
    vector<ConexionGrafo> aristas;
    vector<bool> esTerminal;
    vector<int> padre, grado, xr, pila;
    vector<long long> suma;
    long long costoSinPoda = 0;

    explicit Evaluador(const Grafo& g)
        : grafo(g), aristas(g.conexiones()), esTerminal(g.cantidadNodos(), false),
          padre(g.cantidadNodos()), grado(g.cantidadNodos()), xr(g.cantidadNodos()),
          suma(g.cantidadNodos()) {
        for (int t : g.terminales()) esTerminal[t] = true;
        // Python sorted(..., key=costo) conserva el orden de entrada en empates.
        stable_sort(aristas.begin(), aristas.end(), [](const auto& a, const auto& b) {
            return a.costo < b.costo;
        });
    }

    int buscar(int x) {
        while (padre[x] != x) {
            padre[x] = padre[padre[x]];
            x = padre[x];
        }
        return x;
    }

    long long evaluar(const vector<bool>& activo, vector<bool>& salida,
                      vector<ConexionGrafo>* arbol = nullptr) {
        int cuenta = 0;
        for (int i = 0; i < grafo.cantidadNodos(); ++i) {
            if (!activo[i]) continue;
            padre[i] = i;
            grado[i] = xr[i] = 0;
            suma[i] = 0;
            ++cuenta;
        }
        int usadas = 0;
        long long costo = 0;
        if (arbol) arbol->clear();
        for (const auto& e : aristas) {
            if (usadas >= cuenta - 1) break;
            const int u = e.origen, v = e.destino;
            if (!activo[u] || !activo[v]) continue;
            const int a = buscar(u), b = buscar(v);
            if (a == b) continue;
            padre[a] = b;
            ++usadas;
            costo += e.costo;
            ++grado[u]; ++grado[v];
            xr[u] ^= v; xr[v] ^= u;
            suma[u] += e.costo; suma[v] += e.costo;
            if (arbol) arbol->push_back(e);
        }
        if (usadas != cuenta - 1) return -1;
        costoSinPoda = costo;
        salida = activo;
        pila.clear();
        for (int i = 0; i < grafo.cantidadNodos(); ++i)
            if (activo[i] && grado[i] == 1 && !esTerminal[i]) pila.push_back(i);
        while (!pila.empty()) {
            const int h = pila.back(); pila.pop_back();
            if (!salida[h]) continue;
            const int vecino = xr[h];
            costo -= suma[h];
            salida[h] = false;
            --grado[vecino];
            xr[vecino] ^= h;
            suma[vecino] -= suma[h];
            if (grado[vecino] == 1 && !esTerminal[vecino]) pila.push_back(vecino);
        }
        return costo;
    }
};

vector<bool> caminoMinimoDesdeArbol(const Grafo& grafo, int inicio) {
    const int n = grafo.cantidadNodos();
    vector<bool> enArbol(n, false);
    enArbol[inicio] = true;
    vector<long long> dist(n, numeric_limits<long long>::max());
    vector<int> anterior(n, -1);
    using Entrada = pair<long long, int>;
    priority_queue<Entrada, vector<Entrada>, greater<Entrada>> cola;
    const auto relajar = [&](const vector<int>& fuentes) {
        for (int s : fuentes) { dist[s] = 0; cola.emplace(0, s); }
        while (!cola.empty()) {
            const auto [d, u] = cola.top(); cola.pop();
            if (d > dist[u]) continue;
            for (const auto& e : grafo.vecinos(u)) {
                if (d + e.costo < dist[e.destino]) {
                    dist[e.destino] = d + e.costo;
                    anterior[e.destino] = u;
                    cola.emplace(dist[e.destino], e.destino);
                }
            }
        }
    };
    relajar({inicio});
    // Un orden explícito sustituye la iteración del set de enteros de Python.
    vector<int> terminales = grafo.terminales();
    sort(terminales.begin(), terminales.end());
    while (true) {
        int t = -1;
        for (int candidato : terminales) {
            if (!enArbol[candidato] && (t == -1 || dist[candidato] < dist[t])) t = candidato;
        }
        if (t == -1) break;
        if (dist[t] == numeric_limits<long long>::max())
            throw invalid_argument("Los terminales están desconectados.");
        vector<int> nuevos;
        for (int x = t; !enArbol[x]; x = anterior[x]) {
            enArbol[x] = true;
            nuevos.push_back(x);
        }
        relajar(nuevos);
    }
    return enArbol;
}

size_t elegir(mt19937& azar, size_t cantidad) {
    return uniform_int_distribution<size_t>(0, cantidad - 1)(azar);
}
double aleatorio(mt19937& azar) {
    return generate_canonical<double, 53>(azar);
}
} // namespace

ResultadoRecocidoSteiner ejecutarRecocidoSimuladoSteiner(
    const Grafo& grafo, VarianteRecocido variante, double tiempoLimite, uint32_t semilla,
    const function<void(const ProgresoRecocido&)>& alProgresar) {
    if (!isfinite(tiempoLimite) || tiempoLimite < 0 || grafo.terminales().empty())
        throw invalid_argument("El recocido necesita terminales y un tiempo finito no negativo.");
    if (variante != VarianteRecocido::PorTiempo && variante != VarianteRecocido::PorBloques)
        throw invalid_argument("Variante de recocido desconocida.");
    for (const auto& e : grafo.conexiones())
        if (e.costo < 0 || e.origen == e.destino)
            throw invalid_argument("El recocido necesita costos no negativos y aristas sin bucles.");

    Evaluador evaluador(grafo);
    const int n = grafo.cantidadNodos();
    vector<bool> salida(n), mejor(n);
    const long long costoPodado = evaluador.evaluar(vector<bool>(n, true), salida);
    if (costoPodado < 0) throw invalid_argument("El MST base necesita un grafo conexo.");
    const long long costoMST = evaluador.costoSinPoda;

    mt19937 azar(semilla);
    long long costoMejor = numeric_limits<long long>::max();
    for (int i = 0; i < 30; ++i) {
        const int inicio = grafo.terminales()[elegir(azar, grafo.terminales().size())];
        const auto nodos = caminoMinimoDesdeArbol(grafo, inicio);
        const long long costo = evaluador.evaluar(nodos, salida);
        if (costo < 0) throw runtime_error("Construcción inicial desconectada.");
        if (costo < costoMejor) { costoMejor = costo; mejor = salida; }
    }
    const long long costoInicial = costoMejor;
    vector<bool> actual = mejor, nuevo(n);
    long long costoActual = costoMejor;
    vector<int> lista, steiner;
    const bool porBloques = variante == VarianteRecocido::PorBloques;
    const double tempInicial = 0.3, tempFinal = porBloques ? 0.07 : 0.08;
    constexpr int iterPorBloque = 200000;
    uint64_t iteraciones = 0, bloques = 0;
    const auto inicio = chrono::steady_clock::now();
    const auto segundos = [&] {
        return chrono::duration<double>(chrono::steady_clock::now() - inicio).count();
    };
    const auto avisar = [&] {
        if (alProgresar) alProgresar({iteraciones, bloques, costoMejor, segundos()});
    };
    const auto intentar = [&](double temperatura) {
        lista.clear(); steiner.clear();
        for (int i = 0; i < n; ++i) {
            if (!actual[i]) continue;
            lista.push_back(i);
            if (!evaluador.esTerminal[i]) steiner.push_back(i);
        }
        // principal.py omite TODOS los movimientos si no hay nodos Steiner.
        // secundario.py permite en ese caso el movimiento de agregar.
        if (!porBloques && steiner.empty()) return false;
        const int a = lista[elegir(azar, lista.size())];
        const auto& vecinos = grafo.vecinos(a);
        if (vecinos.empty()) return false; // Caso degenerado: un único nodo.
        const int candidato = vecinos[elegir(azar, vecinos.size())].destino;
        nuevo = actual;
        const double jugada = aleatorio(azar);
        if (jugada < 0.3) {
            if (actual[candidato] || steiner.empty()) return false;
            nuevo[steiner[elegir(azar, steiner.size())]] = false;
            nuevo[candidato] = true;
        } else if (jugada < 0.65) {
            if (steiner.empty()) return false;
            nuevo[steiner[elegir(azar, steiner.size())]] = false;
        } else {
            if (actual[candidato]) return false;
            nuevo[candidato] = true;
        }
        const long long costo = evaluador.evaluar(nuevo, salida);
        if (costo < 0) return false;
        const long long diferencia = costo - costoActual;
        if (diferencia <= 0 || aleatorio(azar) < exp(-static_cast<double>(diferencia) / temperatura)) {
            actual = salida;
            costoActual = costo;
            if (costoActual < costoMejor) { costoMejor = costoActual; mejor = actual; }
        }
        return true;
    };
    while (segundos() < tiempoLimite) {
        if (porBloques) {
            ++bloques;
            const double avance1 = segundos() / tiempoLimite;
            const double avance2 = min(1.0, avance1 + 0.03);
            const double t1 = tempInicial * pow(tempFinal / tempInicial, avance1);
            const double t2 = tempInicial * pow(tempFinal / tempInicial, avance2);
            // Reiniciar la semilla en cada bloque, como np.random.seed(sem).
            azar.seed(static_cast<uint32_t>(semilla * 1000u + bloques));
            for (int it = 0; it < iterPorBloque; ++it) {
                const double temperatura = t1 * pow(t2 / t1, static_cast<double>(it) / iterPorBloque);
                ++iteraciones;
                intentar(temperatura);
            }
            avisar();
        } else {
            ++iteraciones;
            const double avance = segundos() / tiempoLimite;
            const double temperatura = tempInicial * pow(tempFinal / tempInicial, avance);
            if (intentar(temperatura) && iteraciones % 10000 == 0) avisar();
        }
    }
    const double transcurrido = segundos();
    vector<ConexionGrafo> aristasFinales;
    const long long costoFinal = evaluador.evaluar(mejor, salida, &aristasFinales);
    ResultadoSteiner solucion{Grafo(n), salida, costoFinal,
        static_cast<int>(count(salida.begin(), salida.end(), true)), 0};
    for (int t : grafo.terminales()) solucion.arbol.agregarTerminal(t);
    for (const auto& e : aristasFinales) {
        if (!salida[e.origen] || !salida[e.destino]) continue;
        solucion.arbol.agregarArista(e.origen, e.destino, e.costo);
        ++solucion.cantidadAristas;
    }
    const bool valida = validarSteiner(grafo, solucion);
    if (!valida || costoFinal != costoMejor) throw runtime_error("Resultado del recocido inválido.");
    return {move(solucion), costoMST, costoPodado, costoInicial,
            iteraciones, bloques, transcurrido, valida};
}
