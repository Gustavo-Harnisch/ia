#ifndef RECOCIDO_SIMULADO_STEINER_H
#define RECOCIDO_SIMULADO_STEINER_H

#include "Steiner.h"
#include <cstdint>
#include <functional>

// principal.py enfría por tiempo; secundario.py enfría dentro de bloques.
enum class VarianteRecocido { PorTiempo, PorBloques };

struct ProgresoRecocido {
    std::uint64_t iteraciones;
    std::uint64_t bloques;
    long long mejorCosto;
    double segundos;
};

struct ResultadoRecocidoSteiner {
    ResultadoSteiner solucion;
    long long costoMST;
    long long costoMSTPodado;
    long long costoInicial;
    std::uint64_t iteraciones;
    std::uint64_t bloques;
    double tiempoSegundos;
    bool valida;
};

// Traducción de los dos scripts: 30 inicios aleatorios con reemplazo,
// caminos mínimos incrementales, MST inducido + poda y recocido de nodos.
// PorTiempo: 0.3 -> 0.08. PorBloques: 0.3 -> 0.07, 200000 intentos/bloque.
// El límite mide solamente el recocido; PorBloques completa el último bloque.
// Cero segundos permite obtener únicamente la construcción inicial.
// Se conserva el algoritmo, no la secuencia aleatoria de Python/Numba.
ResultadoRecocidoSteiner ejecutarRecocidoSimuladoSteiner(
    const Grafo& grafo, VarianteRecocido variante = VarianteRecocido::PorBloques,
    double tiempoLimite = 600, std::uint32_t semilla = 1,
    const std::function<void(const ProgresoRecocido&)>& alProgresar = {});

#endif
