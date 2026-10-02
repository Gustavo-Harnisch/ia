#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <limits>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <random>
#include <cmath>

#include "Grafo.h"
#include "Lectura.h"
#include "MST.h"
#include "Steiner.h"
#include "Heuristica.h"
#include "BusquedaLocal.h"
#include "IntercambioCaminos.h"
#include "Evolutivo.h"
#include "Grafica.h"
#include "RecocidoSimuladoSteiner.h"


using namespace std;

namespace {
int ejecutarModoRecocido(int argc, char* argv[]) {
    if (argc > 5)
        throw invalid_argument("Uso: proyecto --recocido-tiempo|--recocido-bloques [archivo] [segundos] [semilla]");
    const bool bloques = string(argv[1]) == "--recocido-bloques";
    const filesystem::path archivo = argc > 2 ? filesystem::path(argv[2])
        : filesystem::path(PROYECTO_DIR) / "datos/e18.stp";
    double segundos = 600;
    if (argc > 3) {
        size_t leidos = 0;
        segundos = stod(argv[3], &leidos);
        if (leidos != string(argv[3]).size() || !isfinite(segundos) || segundos < 0)
            throw invalid_argument("Los segundos deben ser un número finito no negativo.");
    }
    // principal.py no fija semilla; secundario.py utiliza 1.
    uint32_t semilla = bloques ? 1 : random_device{}();
    if (argc > 4) {
        const string texto = argv[4];
        if (texto.empty() || texto.find_first_not_of("0123456789") != string::npos)
            throw invalid_argument("La semilla debe ser un entero no negativo.");
        const auto valor = stoull(texto);
        if (valor > numeric_limits<uint32_t>::max()) throw invalid_argument("Semilla fuera de rango.");
        semilla = static_cast<uint32_t>(valor);
    }
    const Grafo grafo = leerGrafo(archivo);
    cout << "Recocido simulado sobre nodos de Steiner "
         << (bloques ? "por bloques (secundario.py)" : "por tiempo (principal.py)") << '\n'
         << "Archivo: " << archivo << '\n'
         << "Nodos: " << grafo.cantidadNodos() << '\n'
         << "Aristas: " << grafo.conexiones().size() << '\n'
         << "Terminales: " << grafo.terminales().size() << '\n'
         << "Semilla: " << semilla << " | Límite del recocido: " << segundos << " s\n"
         << "Generando solución inicial (30 inicios aleatorios)..." << endl;
    const auto resultado = ejecutarRecocidoSimuladoSteiner(grafo,
        bloques ? VarianteRecocido::PorBloques : VarianteRecocido::PorTiempo,
        segundos, semilla, [](const ProgresoRecocido& progreso) {
            cout << "Iteraciones: " << progreso.iteraciones << " | Bloques: " << progreso.bloques
                 << " | Mejor costo: " << progreso.mejorCosto
                 << " | Tiempo: " << progreso.segundos << " s" << endl;
        });
    cout << "Costo MST base: " << resultado.costoMST << '\n'
         << "Costo MST podado: " << resultado.costoMSTPodado << '\n'
         << "Costo solución inicial: " << resultado.costoInicial << '\n'
         << "Costo final recocido simulado Steiner: " << resultado.solucion.costo << '\n'
         << "Solución válida (conexa, árbol y todos los terminales): " << (resultado.valida ? "Sí" : "No") << '\n'
         << "Iteraciones: " << resultado.iteraciones << " | Bloques: " << resultado.bloques << '\n'
         << "Tiempo recocido: " << resultado.tiempoSegundos << " s\n";
    const auto reduccion = [&](const char* etiqueta, long long base) {
        cout << etiqueta;
        if (base == 0) cout << "No aplica (costo base cero)\n";
        else cout << fixed << setprecision(2)
                  << 100.0 * (base - resultado.solucion.costo) / base << " %\n";
    };
    reduccion("Reducción respecto al MST base: ", resultado.costoMST);
    reduccion("Reducción respecto al MST podado: ", resultado.costoMSTPodado);
    return resultado.valida ? 0 : 1;
}
}

int main(int argc, char* argv[])
try
{
    if (argc > 1 && (string(argv[1]) == "--recocido-tiempo" || string(argv[1]) == "--recocido-bloques"))
        return ejecutarModoRecocido(argc, argv);

    if (argc > 5) throw invalid_argument("Uso: proyecto [archivo] [generaciones] [semilla] [hilos]");
    auto numero = [](const string& texto, unsigned long long maximo) {
        if (texto.empty() || texto.find_first_not_of("0123456789") != string::npos) {
            throw invalid_argument("Generaciones, semilla e hilos deben ser números enteros no negativos.");
        }
        const auto valor = stoull(texto);
        if (valor > maximo) throw invalid_argument("Parámetro numérico fuera de rango.");
        return valor;
    };
    const int generaciones = argc > 2 ? static_cast<int>(numero(argv[2], numeric_limits<int>::max())) : 0;
    if (argc > 2 && generaciones == 0) throw invalid_argument("Las generaciones deben ser positivas.");
    const uint32_t semilla = argc > 3 ? static_cast<uint32_t>(numero(argv[3], numeric_limits<uint32_t>::max())) : 42;

    const unsigned hilos = argc > 4 ? static_cast<unsigned>(numero(argv[4], numeric_limits<unsigned>::max())) : 0;

    const filesystem::path archivo = argc > 1
        ? filesystem::path(argv[1])
        : filesystem::path(PROYECTO_DIR) / "datos/e18.stp";


    // ============================
    // 1. Lectura del archivo
    // ============================

    cout << "Archivo: " << archivo << endl;


    Grafo grafo = leerGrafo(archivo);

    size_t cantidadAristas = 0;
    for (int nodo = 0; nodo < grafo.cantidadNodos(); ++nodo) {
        cantidadAristas += grafo.vecinos(nodo).size();
    }
    cantidadAristas /= 2;


    cout << "Nodos: "
         << grafo.cantidadNodos()
         << endl;


    cout << "Aristas: "
         << cantidadAristas
         << endl;


    cout << "Terminales: "
         << grafo.terminales().size()
         << endl;



    // ============================
    // 2. MST
    // ============================


    cout << "\nMST base (Kruskal)\n";


    ResultadoMST mst =
        construirMST(grafo);


    cout << "Costo MST: "
         << mst.costo
         << endl;



    // ============================
    // 3. Poda Steiner
    // ============================


    cout << "\nSteiner: MST podado\n";


    ResultadoSteiner steiner =
        podarSteiner(mst.arbol);


    cout << "Costo podado: "
         << steiner.costo
         << endl;



    // ============================
    // 4. Heurística
    // ============================


    cout << "\nHeurística caminos mínimos\n";


    ResultadoSteiner heuristica =
        mejorarPorCaminos(grafo, 0, hilos).solucion;


    cout << "Costo heurística: "
         << heuristica.costo
         << endl;



    // ============================
    // 5. Búsqueda local
    // ============================


    cout << "\nBúsqueda local\n";


    ResultadoSteiner local =
        mejorarBusquedaLocal(grafo, heuristica).solucion;


    cout << "Costo búsqueda local: "
         << local.costo
         << endl;



    // ============================
    // 6. Intercambio de caminos
    // ============================

    cout << "\nIntercambio de caminos\n";
    const auto caminos = mejorarIntercambioCaminos(grafo, local, 10, hilos);
    cout << "Costo intercambio: " << caminos.solucion.costo
         << " | Mejoras: " << caminos.mejorasAceptadas
         << " | Caminos evaluados: " << caminos.caminosEvaluados << endl;

    // ============================
    // 7. Evolutivo elitista
    // ============================


    cout << "\n============================\n";
    cout << "Algoritmo evolutivo elitista\n";
    cout << "============================\n";


    // Una carpeta por ejecución evita mezclar imágenes de procesos simultáneos.
    const auto identificador = chrono::duration_cast<chrono::microseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
    const filesystem::path carpeta = filesystem::path(PROYECTO_DIR) / "salidas"
        / ("evolutivo_" + to_string(identificador) + "_semilla_" + to_string(semilla));
    bool graficosActivos = true;
    const auto guardarMejor = [&](const ResultadoSteiner& mejor, int generacion) {
        if (!graficosActivos) return;
        try {
            generarGraficaSteiner(grafo, mejor, carpeta / "mejor", generacion, semilla);
            ofstream historial(carpeta / "mejoras.csv", ios::app);
            if (generacion == 0) historial << "generacion,costo,nodos,aristas\n";
            historial << generacion << ',' << mejor.costo << ',' << mejor.cantidadNodos
                      << ',' << mejor.cantidadAristas << '\n';
            historial.close();
            if (!historial) throw runtime_error("No se pudo guardar el historial gráfico.");
            cout << "Gráfico actualizado: " << carpeta / "mejor.png" << endl;
        } catch (const exception& error) {
            graficosActivos = false;
            cerr << "No se actualizarán más gráficos: " << error.what()
                 << " La optimización continúa." << endl;
        }
    };
    cout << "Imágenes de esta ejecución: " << carpeta << endl;
    ResultadoEvolutivo evolucion = ejecutarEvolutivoElitista(
        grafo, caminos.solucion, argc > 2 ? generaciones : GENERACIONES_PREDETERMINADAS,
        semilla, guardarMejor, hilos);


    cout << "\nResultado final evolutivo\n";


    cout << "Costo final: "
         << evolucion.costo
         << endl;


    cout << "Solución válida: "
         << (evolucion.valida ? "Sí" : "No")
         << endl;


    cout << "Generaciones: " << evolucion.generaciones << '\n'
         << "Soluciones evaluadas: " << evolucion.solucionesEvaluadas << '\n'
         << "Descendientes diferentes de sus padres: " << evolucion.descendientesDiferentes << '\n'
         << "Cruces: " << evolucion.crucesRealizados << '\n'
         << "Población final sin duplicados: " << evolucion.poblacionFinal << '\n'
         << "Tiempo evolutivo: " << evolucion.tiempoSegundos << " s" << endl;
    return evolucion.valida ? 0 : 1;
}

catch (const exception& error)
{
    cerr << "Error: " << error.what() << endl;
    return 1;
}
