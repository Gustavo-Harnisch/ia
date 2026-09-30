#include "Grafica.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {
// Proteger rutas con espacios o comillas al invocar Graphviz en Linux.
string entreComillas(const string& texto) {
    string resultado = "'";
    for (char caracter : texto) {
        resultado += caracter == '\'' ? "'\"'\"'" : string(1, caracter);
    }
    return resultado + "'";
}
}

void generarGrafica(const Grafo& grafo, const filesystem::path& base) {
    if (!base.parent_path().empty()) {
        filesystem::create_directories(base.parent_path());
    }
    const string archivoDot = base.string() + ".dot";
    ofstream salida(archivoDot);
    if (!salida) {
        throw runtime_error("No se pudo crear: " + archivoDot);
    }

    vector<bool> esTerminal(grafo.cantidadNodos(), false);
    for (int nodo : grafo.terminales()) {
        esTerminal[nodo] = true;
    }

    // DOT describe el dibujo; Graphviz calcula dónde colocar los nodos.
    salida << "graph Grafo {\n"
           << "  graph [bgcolor=\"white\", pad=0.4, nodesep=0.6, ranksep=0.7, "
              "label=\"Grafo de ejemplo\\nAzul: terminal | Gris: opcional\\n"
              "Los números en las conexiones son costos\", "
              "labelloc=t, fontname=\"DejaVu Sans\", fontsize=16];\n"
           << "  node [shape=circle, style=filled, fontname=\"DejaVu Sans\", "
              "fontsize=16, width=0.65, penwidth=2];\n"
           << "  edge [fontname=\"DejaVu Sans\", fontsize=14, color=\"#64748b\", "
              "fontcolor=\"#334155\", penwidth=2];\n";

    // Declarar todos los nodos permite mostrar también los que estén aislados.
    for (int nodo = 0; nodo < grafo.cantidadNodos(); ++nodo) {
        salida << "  " << nodo
               << (esTerminal[nodo]
                       ? " [fillcolor=\"#2563eb\", color=\"#1d4ed8\", fontcolor=white];\n"
                       : " [fillcolor=\"#e2e8f0\", color=\"#94a3b8\", fontcolor=\"#0f172a\"];\n");
    }
    for (int origen = 0; origen < grafo.cantidadNodos(); ++origen) {
        for (const Arista& arista : grafo.vecinos(origen)) {
            // El grafo guarda cada conexión en ambos sentidos; dibujarla una vez.
            if (origen < arista.destino) {
                salida << "  " << origen << " -- " << arista.destino
                       << " [label=\"" << arista.costo << "\"];\n";
            }
        }
    }
    salida << "}\n";
    salida.close();
    if (!salida) {
        throw runtime_error("No se pudo terminar de escribir el archivo DOT.");
    }

    for (const string formato : {"svg", "png"}) {
        const string comando = entreComillas(GRAPHVIZ_DOT) + " -T" + formato
            + " " + entreComillas(archivoDot) + " -o "
            + entreComillas(base.string() + "." + formato);
        if (system(comando.c_str()) != 0) {
            throw runtime_error("Graphviz no pudo generar la imagen " + formato + ".");
        }
    }
}

void generarGraficaSteiner(const Grafo& original, const ResultadoSteiner& solucion,
                          const filesystem::path& base, int generacion, uint32_t semilla) {
    if (!validarSteiner(original, solucion)) {
        throw invalid_argument("No se puede dibujar una solución Steiner inválida.");
    }
    if (!base.parent_path().empty()) filesystem::create_directories(base.parent_path());
    const string temporal = base.string() + ".nuevo";
    ofstream salida(temporal + ".dot");
    if (!salida) throw runtime_error("No se pudo guardar el gráfico en " + base.string());
    vector<bool> terminal(original.cantidadNodos(), false);
    for (int nodo : original.terminales()) terminal[nodo] = true;
    salida << "graph MejorSteiner {\n"
           << " graph [bgcolor=white, overlap=prism, splines=line, start=42, pad=0.5, "
              "labelloc=t, fontname=\"DejaVu Sans\", fontsize=22, label=\"Mejor solución de Steiner"
           << "\\nCosto: " << solucion.costo << " | Generación: " << generacion
           << " | Semilla: " << semilla << "\\n" << solucion.cantidadNodos
           << " nodos | " << solucion.cantidadAristas << " aristas | " << original.terminales().size()
           << " terminales\\nAzul: terminal obligatorio | Naranja: nodo Steiner"
              "\\nAristas: costo original | Identificadores internos: base 0\"];\n"
           << " node [shape=circle, style=filled, fontname=\"DejaVu Sans\", fontsize=10, "
              "width=0.35, margin=0.03, penwidth=1.3];\n"
           << " edge [color=\"#64748b\", fontcolor=\"#334155\", fontname=\"DejaVu Sans\", fontsize=8];\n";
    for (int nodo = 0; nodo < original.cantidadNodos(); ++nodo) {
        if (!solucion.activos[nodo]) continue;
        salida << ' ' << nodo << (terminal[nodo]
            ? " [fillcolor=\"#2563eb\", color=\"#1d4ed8\", fontcolor=white];\n"
            : " [fillcolor=\"#fed7aa\", color=\"#ea580c\", fontcolor=\"#431407\"];\n");
        for (const auto& arista : solucion.arbol.vecinos(nodo)) {
            if (nodo < arista.destino) {
                salida << ' ' << nodo << " -- " << arista.destino
                       << " [xlabel=\"" << arista.costo << "\", tooltip=\"Costo: " << arista.costo << "\"];\n";
            }
        }
    }
    salida << "}\n";
    salida.close();
    if (!salida) throw runtime_error("No se pudo completar el archivo DOT.");
    // sfdp distribuye árboles grandes sin crear una imagen extremadamente alta.
    // El SVG conserva el detalle; el PNG tiene un tamaño acotado para compartirlo.
    for (const string formato : {"svg", "png"}) {
        const string comando = entreComillas(GRAPHVIZ_DOT) + " -Ksfdp -T" + formato
            + (formato == "png" ? " -Gsize=24,24 -Gdpi=100" : "")
            + " " + entreComillas(temporal + ".dot")
            + " -o " + entreComillas(temporal + "." + formato);
        if (system(comando.c_str()) != 0) throw runtime_error("No se pudo dibujar la mejor solución en " + formato);
    }
    for (const string extension : {"dot", "svg", "png"}) {
        filesystem::rename(temporal + "." + extension, base.string() + "." + extension);
    }
}
