#include "Evolutivo.h"
#include "Heuristica.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <tuple>

using namespace std;

namespace {
struct Conexion {
    int origen, destino, costo;
};
using Firma = vector<tuple<int, int, int>>;

Firma firma(const ResultadoSteiner& solucion) {
    Firma aristas;
    for (int nodo = 0; nodo < solucion.arbol.cantidadNodos(); ++nodo) {
        for (const auto& arista : solucion.arbol.vecinos(nodo)) {
            if (nodo < arista.destino) aristas.emplace_back(nodo, arista.destino, arista.costo);
        }
    }
    sort(aristas.begin(), aristas.end());
    // Con un solo terminal no hay aristas, pero tampoco árboles alternativos.
    return aristas;
}

vector<Conexion> conexionesDel(const Grafo& grafo) {
    vector<Conexion> conexiones;
    for (int nodo = 0; nodo < grafo.cantidadNodos(); ++nodo) {
        for (const auto& arista : grafo.vecinos(nodo)) {
            if (arista.costo < 0) throw invalid_argument("El evolutivo necesita costos no negativos.");
            if (nodo < arista.destino) conexiones.push_back({nodo, arista.destino, arista.costo});
        }
    }
    return conexiones;
}

int representante(vector<int>& padre, int nodo) {
    while (padre[nodo] != nodo) {
        padre[nodo] = padre[padre[nodo]];
        nodo = padre[nodo];
    }
    return nodo;
}

// La unión de dos árboles factibles es conexa porque comparten los terminales.
// Kruskal elimina ciclos; barajar antes de ordenar varía los empates de costo.
// Se utilizan siempre los costos originales, sin alterar la función objetivo.
ResultadoSteiner reconstruir(const Grafo& grafo, const vector<Conexion>& conexiones,
                            const vector<bool>& activos, mt19937& azar) {
    const int n = grafo.cantidadNodos();
    vector<int> originalALocal(n, -1), localAOriginal;
    for (int nodo = 0; nodo < n; ++nodo) {
        if (activos[nodo]) {
            originalALocal[nodo] = static_cast<int>(localAOriginal.size());
            localAOriginal.push_back(nodo);
        }
    }
    vector<Conexion> candidatas;
    for (const auto& arista : conexiones) {
        if (activos[arista.origen] && activos[arista.destino]) candidatas.push_back(arista);
    }
    shuffle(candidatas.begin(), candidatas.end(), azar);
    stable_sort(candidatas.begin(), candidatas.end(), [](const Conexion& a, const Conexion& b) {
        return a.costo < b.costo;
    });

    const int cantidad = static_cast<int>(localAOriginal.size());
    Grafo arbol(cantidad);
    for (int terminal : grafo.terminales()) arbol.agregarTerminal(originalALocal[terminal]);
    vector<int> padre(n), tamanio(n, 1);
    iota(padre.begin(), padre.end(), 0);
    int elegidas = 0;
    for (const auto& arista : candidatas) {
        int a = representante(padre, arista.origen);
        int b = representante(padre, arista.destino);
        if (a == b) continue;
        if (tamanio[a] < tamanio[b]) swap(a, b);
        padre[b] = a;
        tamanio[a] += tamanio[b];
        arbol.agregarArista(originalALocal[arista.origen], originalALocal[arista.destino], arista.costo);
        if (++elegidas == cantidad - 1) break;
    }
    if (elegidas != cantidad - 1) throw runtime_error("El cruce o mutación desconectó la selección.");
    const auto podado = podarSteiner(arbol);
    ResultadoSteiner resultado{Grafo(n), vector<bool>(n, false), podado.costo,
                              podado.cantidadNodos, podado.cantidadAristas};
    for (int terminal : grafo.terminales()) resultado.arbol.agregarTerminal(terminal);
    for (int local = 0; local < cantidad; ++local) {
        const int nodo = localAOriginal[local];
        resultado.activos[nodo] = podado.activos[local];
        for (const auto& arista : podado.arbol.vecinos(local)) {
            if (local < arista.destino) {
                resultado.arbol.agregarArista(nodo, localAOriginal[arista.destino], arista.costo);
            }
        }
    }
    return resultado;
}

// Incorporar opcionales conectados crea caminos alternativos. La reconstrucción
// puede cambiar conexiones y la poda puede retirar opcionales de ambos padres.
ResultadoSteiner descendiente(const Grafo& grafo, const vector<Conexion>& conexiones,
                              const ResultadoSteiner& padre, const ResultadoSteiner* madre,
                              mt19937& azar) {
    vector<bool> activos = padre.activos;
    if (madre) {
        for (int nodo = 0; nodo < grafo.cantidadNodos(); ++nodo) {
            activos[nodo] = activos[nodo] || madre->activos[nodo];
        }
    }
    vector<int> opcionales;
    for (int nodo = 0; nodo < grafo.cantidadNodos(); ++nodo) {
        if (!activos[nodo]) opcionales.push_back(nodo);
    }
    shuffle(opcionales.begin(), opcionales.end(), azar);
    const int cambios = uniform_int_distribution<int>(1, 4)(azar);
    int agregados = 0;
    for (int nodo : opcionales) {
        // Dos vecinos distintos permiten introducir una ruta alternativa.
        int primero = -1;
        bool conecta = false;
        for (const auto& arista : grafo.vecinos(nodo)) {
            if (!activos[arista.destino]) continue;
            if (primero == -1) primero = arista.destino;
            else if (primero != arista.destino) { conecta = true; break; }
        }
        if (conecta) {
            activos[nodo] = true;
            if (++agregados == cambios) break;
        }
    }
    return reconstruir(grafo, conexiones, activos, azar);
}

size_t torneo(const vector<ResultadoSteiner>& poblacion, mt19937& azar) {
    uniform_int_distribution<size_t> elegir(0, poblacion.size() - 1);
    const size_t a = elegir(azar), b = elegir(azar);
    return poblacion[a].costo < poblacion[b].costo ? a : b;
}
}

long long calcularCostoEvolutivo(const ResultadoSteiner& solucion) { return solucion.costo; }

bool validarConectividadEvolutivo(const Grafo& grafo, const ResultadoSteiner& solucion) {
    return validarSteiner(grafo, solucion);
}

ResultadoSteiner mutarSolucion(const Grafo& grafo, const ResultadoSteiner& solucion, uint32_t semilla) {
    if (!validarSteiner(grafo, solucion)) throw invalid_argument("La mutación necesita una solución válida.");
    mt19937 azar(semilla);
    auto resultado = descendiente(grafo, conexionesDel(grafo), solucion, nullptr, azar);
    if (!validarSteiner(grafo, resultado)) throw runtime_error("Mutación inválida.");
    return resultado;
}

void monitorEvolutivo(EstadoEvolutivo& estado) {
    while (estado.ejecutando) {
        for (int paso = 0; paso < 100 && estado.ejecutando; ++paso) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        if (!estado.ejecutando) break;
        cout << "\nMONITOR EVOLUTIVO | Generación: " << estado.generacion
             << " | Evaluadas: " << estado.evaluadas
             << " | Diferentes de sus padres: " << estado.diferentes
             << " | Población distinta: " << estado.poblacion
             << " | Mejor costo: " << estado.mejorCosto << endl;
    }
}

ResultadoEvolutivo ejecutarEvolutivoElitista(
    const Grafo& grafo, const ResultadoSteiner& solucionInicial,
    int maxGeneraciones, uint32_t semilla, const ObservadorMejora& alMejorar) {
    if (maxGeneraciones <= 0) throw invalid_argument("El número de generaciones debe ser positivo.");
    if (!validarSteiner(grafo, solucionInicial)) throw invalid_argument("La solución inicial no es válida.");
    const auto conexiones = conexionesDel(grafo);
    const auto inicio = chrono::steady_clock::now();
    mt19937 azar(semilla);
    const size_t maxPoblacion = 16;
    const size_t hilos = max(1u, min(4u, thread::hardware_concurrency()));
    cout << "\nEvolutivo: " << maxGeneraciones << " generaciones, semilla " << semilla
         << ", hasta " << maxPoblacion << " individuos y " << hilos << " tareas simultáneas.\n";
    vector<ResultadoSteiner> poblacion{solucionInicial};
    ResultadoSteiner mejor = solucionInicial;
    ResultadoEvolutivo resultado{};
    resultado.historialCostos.push_back(mejor.costo);
    EstadoEvolutivo estado;
    estado.mejorCosto = mejor.costo;
    if (alMejorar) alMejorar(mejor, 0);
    thread monitor(monitorEvolutivo, ref(estado));
    try {
        for (int generacion = 0; generacion < maxGeneraciones; ++generacion) {
            vector<ResultadoSteiner> candidatos = poblacion; // Sobreviven los padres, incluida la élite.
            vector<Firma> firmasPadres;
            for (const auto& individuo : poblacion) firmasPadres.push_back(firma(individuo));
            for (size_t lote = 0; lote < maxPoblacion; lote += hilos) {
                vector<future<ResultadoSteiner>> futuros;
                vector<pair<size_t, size_t>> padres;
                for (size_t tarea = lote; tarea < min(lote + hilos, maxPoblacion); ++tarea) {
                    const size_t a = torneo(poblacion, azar);
                    size_t b = a;
                    if (tarea % 2 == 0 && poblacion.size() > 1) {
                        b = (a + uniform_int_distribution<size_t>(1, poblacion.size() - 1)(azar)) % poblacion.size();
                        ++resultado.crucesRealizados;
                    }
                    padres.emplace_back(a, b);
                    const uint32_t semillaHijo = azar();
                    // Cada tarea tiene su propio generador; el orden de los hilos no altera el resultado.
                    futuros.push_back(async(launch::async, [&, a, b, semillaHijo]() {
                        mt19937 azarHijo(semillaHijo);
                        return descendiente(grafo, conexiones, poblacion[a],
                                            a == b ? nullptr : &poblacion[b], azarHijo);
                    }));
                }
                for (size_t i = 0; i < futuros.size(); ++i) {
                    auto hijo = futuros[i].get();
                    ++estado.evaluadas;
                    if (!validarSteiner(grafo, hijo)) throw runtime_error("Descendiente inválido.");
                    const auto identidad = firma(hijo);
                    const auto [a, b] = padres[i];
                    if (identidad != firmasPadres[a] && identidad != firmasPadres[b]) ++estado.diferentes;
                    candidatos.push_back(move(hijo));
                }
            }
            // Nuevos inicios permiten explorar nodos ausentes en toda la población.
            if (generacion % 10 == 0) {
                const auto& terminales = grafo.terminales();
                const int terminal = terminales[uniform_int_distribution<size_t>(0, terminales.size() - 1)(azar)];
                auto inmigrante = construirPorCaminos(grafo, terminal);
                ++estado.evaluadas;
                if (!validarSteiner(grafo, inmigrante)) throw runtime_error("Inmigrante inválido.");
                candidatos.push_back(move(inmigrante));
            }
            // Aleatorizar empates impide favorecer siempre a los padres de igual costo.
            shuffle(candidatos.begin(), candidatos.end(), azar);
            stable_sort(candidatos.begin(), candidatos.end(), [](const auto& a, const auto& b) {
                return a.costo < b.costo;
            });
            if (candidatos.front().costo < mejor.costo) {
                mejor = candidatos.front();
                estado.mejorCosto = mejor.costo;
                cout << "Generación " << generacion + 1 << ": nuevo mejor costo " << mejor.costo << endl;
                if (alMejorar) alMejorar(mejor, generacion + 1);
            }
            set<Firma> vistas;
            vector<ResultadoSteiner> distintas;
            for (auto& candidato : candidatos) {
                if (vistas.insert(firma(candidato)).second) distintas.push_back(move(candidato));
            }
            // Las mejores 14 son élite; dos plazas exploran soluciones distintas,
            // incluso de mayor costo, para evitar que la población sea solo clones.
            if (distintas.size() > maxPoblacion) {
                shuffle(distintas.begin() + maxPoblacion - 2, distintas.end(), azar);
                distintas.erase(distintas.begin() + maxPoblacion, distintas.end());
            }
            poblacion = move(distintas);
            estado.poblacion = static_cast<int>(poblacion.size());
            estado.generacion = generacion + 1;
            resultado.historialCostos.push_back(mejor.costo);
        }
    } catch (...) {
        estado.ejecutando = false;
        monitor.join();
        throw;
    }
    estado.ejecutando = false;
    monitor.join();
    resultado.costo = mejor.costo;
    resultado.valida = validarSteiner(grafo, mejor);
    resultado.generaciones = estado.generacion;
    resultado.solucionesEvaluadas = estado.evaluadas;
    resultado.descendientesDiferentes = estado.diferentes;
    resultado.poblacionFinal = estado.poblacion;
    resultado.semilla = semilla;
    resultado.tiempoSegundos = chrono::duration<double>(chrono::steady_clock::now() - inicio).count();
    for (int nodo = 0; nodo < grafo.cantidadNodos(); ++nodo) {
        if (!mejor.activos[nodo]) continue;
        resultado.nodos.push_back(nodo);
        for (const auto& arista : mejor.arbol.vecinos(nodo)) {
            if (nodo < arista.destino) resultado.aristas.emplace_back(nodo, arista.destino);
        }
    }
    return resultado;
}
