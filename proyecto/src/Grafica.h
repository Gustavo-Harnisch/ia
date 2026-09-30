#ifndef GRAFICA_H
#define GRAFICA_H

#include "Grafo.h"
#include "Steiner.h"
#include <cstdint>
#include <filesystem>

// Por ejemplo, la base salidas/grafo genera grafo.dot, grafo.svg y grafo.png.
void generarGrafica(const Grafo& grafo, const std::filesystem::path& base);

// Dibuja exclusivamente nodos y aristas de la solución validada.
// Publica cada archivo mediante rename para evitar imágenes a medio escribir.
void generarGraficaSteiner(const Grafo& original, const ResultadoSteiner& solucion,
    const std::filesystem::path& base, int generacion, std::uint32_t semilla);

#endif
