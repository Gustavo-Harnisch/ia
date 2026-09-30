#ifndef LECTURA_H
#define LECTURA_H

#include "Grafo.h"
#include <filesystem>

// Detecta el TXT de aprendizaje o SteinLib STP 1.0 no dirigido (como E18).
// Convierte los identificadores de SteinLib de base 1 a base 0.
Grafo leerGrafo(const std::filesystem::path& archivo);

#endif
