#ifndef EVOLUTIVO_H
#define EVOLUTIVO_H

#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>
#include <utility>
#include <functional>

#include "Grafo.h"
#include "Steiner.h"


// Resultado final del algoritmo evolutivo
struct ResultadoEvolutivo
{
    long long costo;

    bool valida;

    int generaciones;

    int solucionesEvaluadas;

    double tiempoSegundos;

    int descendientesDiferentes;
    int crucesRealizados;
    int poblacionFinal;
    std::uint32_t semilla;
    std::vector<long long> historialCostos;

    std::vector<int> nodos;

    std::vector<std::pair<int,int>> aristas;
};


// Información compartida para el monitor
struct EstadoEvolutivo
{
    std::atomic<long long> mejorCosto;

    std::atomic<int> generacion;

    std::atomic<int> evaluadas;

    std::atomic<int> diferentes{0};
    std::atomic<int> poblacion{1};

    std::atomic<bool> ejecutando;


    EstadoEvolutivo()
        :
        mejorCosto(999999999),
        generacion(0),
        evaluadas(0),
        ejecutando(true)
    {}
};


// Notifica la solución inicial (generación 0) y cada mejora estricta.
using ObservadorMejora = std::function<void(const ResultadoSteiner&, int)>;
inline constexpr int GENERACIONES_PREDETERMINADAS = 1000000;

// Ejecuta el algoritmo evolutivo elitista
ResultadoEvolutivo ejecutarEvolutivoElitista(
    const Grafo& grafo,
    const ResultadoSteiner& solucionInicial,
    int maxGeneraciones = GENERACIONES_PREDETERMINADAS,
    std::uint32_t semilla = 42,
    const ObservadorMejora& alMejorar = {}
);


// Evalúa si una solución mantiene todos los terminales conectados
bool validarConectividadEvolutivo(
    const Grafo& grafo,
    const ResultadoSteiner& solucion
);


// Calcula el costo de una solución
long long calcularCostoEvolutivo(
    const ResultadoSteiner& solucion
);


// Genera una mutación de una solución Steiner
ResultadoSteiner mutarSolucion(
    const Grafo& grafo,
    const ResultadoSteiner& solucion,
    std::uint32_t semilla = 42
);


// Monitor cada 10 segundos
void monitorEvolutivo(
    EstadoEvolutivo& estado
);


#endif
