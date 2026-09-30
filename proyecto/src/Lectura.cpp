#include "Lectura.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

namespace {
int leerCantidad(istream& entrada, const string& seccion) {
    string etiqueta;
    int cantidad;
    if (!(entrada >> etiqueta >> cantidad) || etiqueta != seccion || cantidad < 0) {
        throw runtime_error("Se esperaba " + seccion + " seguido de una cantidad no negativa.");
    }
    return cantidad;
}

Grafo leerEjemplo(istream& entrada) {
    const int nodos = leerCantidad(entrada, "NODOS");
    Grafo grafo(nodos);
    const int terminales = leerCantidad(entrada, "TERMINALES");
    if (terminales > nodos) {
        throw runtime_error("No puede haber más terminales que nodos.");
    }
    for (int i = 0; i < terminales; ++i) {
        int nodo;
        if (!(entrada >> nodo)) {
            throw runtime_error("Falta un identificador de terminal válido.");
        }
        // Grafo comprueba que el identificador esté entre 0 y NODOS - 1.
        grafo.agregarTerminal(nodo);
    }
    if (static_cast<int>(grafo.terminales().size()) != terminales) {
        throw runtime_error("Hay terminales repetidos.");
    }

    const int aristas = leerCantidad(entrada, "ARISTAS");
    for (int i = 0; i < aristas; ++i) {
        int origen, destino, costo;
        if (!(entrada >> origen >> destino >> costo)) {
            throw runtime_error("Cada arista debe indicar origen, destino y costo.");
        }
        if (costo < 0 || origen == destino) {
            throw runtime_error("Usa costos no negativos y conexiones entre nodos diferentes.");
        }
        grafo.agregarArista(origen, destino, costo);
    }

    string sobrante;
    if (entrada >> sobrante) {
        throw runtime_error("Hay datos adicionales: revisa las cantidades declaradas.");
    }
    return grafo;
}

// Lector del formato STP no dirigido usado por E18.
// Cada sección tiene una cantidad declarada que verificamos al terminar.
Grafo leerSteinLib(istream& entrada) {
    string linea;
    getline(entrada, linea);
    if (!linea.empty() && linea.back() == '\r') linea.pop_back();
    if (linea != "33D32945 STP File, STP Format Version 1.0") {
        throw runtime_error("Cabecera SteinLib no reconocida: se admite STP 1.0.");
    }

    optional<Grafo> grafo;
    string seccion;
    int aristasEsperadas = -1, terminalesEsperados = -1;
    int aristasLeidas = 0, terminalesLeidos = 0;
    bool grafoCerrado = false, terminalesCerrados = false, fin = false;
    int numeroLinea = 1;

    while (getline(entrada, linea)) {
        ++numeroLinea;
        istringstream datos(linea);
        string clave;
        if (!(datos >> clave) || clave[0] == '#') continue;

        auto error = [&](const string& mensaje) {
            throw runtime_error("Línea " + to_string(numeroLinea) + ": " + mensaje);
        };
        if (fin) error("Hay contenido después de EOF.");

        if (clave == "SECTION") {
            if (!seccion.empty()) error("Falta END antes de la siguiente sección.");
            if (!(datos >> seccion)) error("Falta el nombre de la sección.");
            if (seccion == "Graph") {
                if (grafo || grafoCerrado) error("La sección Graph está repetida.");
            } else if (seccion == "Terminals") {
                if (!grafoCerrado || terminalesCerrados) error("Terminals debe aparecer una vez, después de Graph.");
            } else if (seccion != "Comment") {
                error("Sección no admitida: " + seccion);
            }
        } else if (clave == "END") {
            if (seccion.empty()) error("END sin sección abierta.");
            if (seccion == "Graph") {
                if (!grafo || aristasEsperadas < 0 || aristasLeidas != aristasEsperadas) {
                    error("La cantidad de aristas no coincide o faltan Nodes/Edges.");
                }
                grafoCerrado = true;
            } else if (seccion == "Terminals") {
                if (terminalesEsperados < 0 || terminalesLeidos != terminalesEsperados) {
                    error("La cantidad de terminales no coincide o falta Terminals.");
                }
                if (static_cast<int>(grafo->terminales().size()) != terminalesLeidos) {
                    error("Hay terminales repetidos.");
                }
                terminalesCerrados = true;
            }
            seccion.clear();
        } else if (clave == "EOF") {
            if (!seccion.empty() || !grafoCerrado || !terminalesCerrados) {
                error("Faltan secciones completas antes de EOF.");
            }
            fin = true;
        } else if (seccion == "Comment") {
            continue; // Nombre, autor y descripción no modifican el grafo.
        } else if (seccion == "Graph") {
            if (clave == "Nodes") {
                int cantidad;
                if (grafo || !(datos >> cantidad) || cantidad <= 0) error("Nodes debe indicar una cantidad positiva una sola vez.");
                grafo.emplace(cantidad);
            } else if (clave == "Edges") {
                if (aristasEsperadas >= 0 || !(datos >> aristasEsperadas) || aristasEsperadas < 0) {
                    error("Edges debe indicar una cantidad no negativa una sola vez.");
                }
            } else if (clave == "E") {
                int origen, destino, costo;
                if (!grafo || aristasEsperadas < 0 || !(datos >> origen >> destino >> costo)) {
                    error("Se esperaba E origen destino costo después de Nodes y Edges.");
                }
                if (origen < 1 || origen > grafo->cantidadNodos() || destino < 1 || destino > grafo->cantidadNodos()) {
                    error("Los identificadores deben estar entre 1 y Nodes.");
                }
                if (origen == destino || costo < 0) error("Conexión inválida: usa nodos distintos y costo no negativo.");
                // SteinLib empieza en 1; nuestros vectores empiezan en 0.
                grafo->agregarArista(origen - 1, destino - 1, costo);
                if (++aristasLeidas > aristasEsperadas) error("Hay más aristas de las declaradas.");
            } else {
                error("Registro de grafo no admitido: " + clave);
            }
        } else if (seccion == "Terminals") {
            if (clave == "Terminals") {
                if (terminalesEsperados >= 0 || !(datos >> terminalesEsperados)
                    || terminalesEsperados < 0 || terminalesEsperados > grafo->cantidadNodos()) {
                    error("Cantidad de terminales inválida o repetida.");
                }
            } else if (clave == "T") {
                int nodo;
                if (terminalesEsperados < 0 || !(datos >> nodo) || nodo < 1 || nodo > grafo->cantidadNodos()) {
                    error("Terminal inválido: se esperaba T identificador después de Terminals.");
                }
                grafo->agregarTerminal(nodo - 1);
                if (++terminalesLeidos > terminalesEsperados) error("Hay más terminales de los declarados.");
            } else {
                error("Registro de terminal no admitido: " + clave);
            }
        } else {
            error("Registro fuera de una sección.");
        }

        string sobrante;
        if (datos >> sobrante && sobrante[0] != '#') error("Hay datos adicionales en la línea.");
    }
    if (!fin) throw runtime_error("El archivo SteinLib está incompleto: falta EOF.");
    return *grafo;
}
} // namespace: funciones auxiliares privadas de este archivo.

Grafo leerGrafo(const filesystem::path& archivo) {
    ifstream entrada(archivo);
    if (!entrada) throw runtime_error("No se pudo abrir: " + archivo.string());

    // Reconocer el contenido permite mantener el TXT de aprendizaje y el STP.
    string primeraPalabra;
    entrada >> primeraPalabra;
    entrada.clear();
    entrada.seekg(0);
    if (primeraPalabra == "NODOS") return leerEjemplo(entrada);
    if (primeraPalabra == "33D32945") return leerSteinLib(entrada);
    throw runtime_error("Formato no reconocido. Usa el TXT de ejemplo o SteinLib STP 1.0.");
}
