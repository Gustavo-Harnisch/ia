#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <limits>
#include <string>
#include <chrono>
#include <fstream>

#include "Grafo.h"
#include "Lectura.h"
#include "MST.h"
#include "Steiner.h"
#include "Heuristica.h"
#include "BusquedaLocal.h"
#include "Evolutivo.h"
#include "Grafica.h"


using namespace std;


int main(int argc, char* argv[])
try
{

    if (argc > 4) throw invalid_argument("Uso: proyecto [archivo] [generaciones] [semilla]");
    auto numero = [](const string& texto, unsigned long long maximo) {
        if (texto.empty() || texto.find_first_not_of("0123456789") != string::npos) {
            throw invalid_argument("Generaciones y semilla deben ser números enteros no negativos.");
        }
        const auto valor = stoull(texto);
        if (valor > maximo) throw invalid_argument("Parámetro numérico fuera de rango.");
        return valor;
    };
    const int generaciones = argc > 2 ? static_cast<int>(numero(argv[2], numeric_limits<int>::max())) : 0;
    if (argc > 2 && generaciones == 0) throw invalid_argument("Las generaciones deben ser positivas.");
    const uint32_t semilla = argc > 3 ? static_cast<uint32_t>(numero(argv[3], numeric_limits<uint32_t>::max())) : 42;

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
        mejorarPorCaminos(grafo).solucion;


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
    // 6. Evolutivo elitista
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
        grafo, local, argc > 2 ? generaciones : GENERACIONES_PREDETERMINADAS,
        semilla, guardarMejor);


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
