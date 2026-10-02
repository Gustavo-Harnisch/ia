#include "Paralelismo.h"
#include <algorithm>
#include <stdexcept>

unsigned resolverHilos(unsigned solicitados, unsigned tareas) {
    const unsigned disponibles = std::max(1u, std::thread::hardware_concurrency());
    return std::max(1u, std::min(solicitados == 0 ? disponibles : solicitados, tareas));
}

EjecutorParalelo::EjecutorParalelo(unsigned hilos) {
    if (hilos == 0) throw std::invalid_argument("El ejecutor necesita al menos un hilo.");
    // Un hilo ejecuta directamente en el llamador, sin sincronización adicional.
    if (hilos == 1) return;
    try {
        for (unsigned i = 0; i < hilos; ++i) trabajadores.emplace_back([this] { trabajar(); });
    } catch (...) {
        detener();
        throw;
    }
}

void EjecutorParalelo::detener() {
    {
        std::lock_guard<std::mutex> bloqueo(mutex);
        cerrando = true;
    }
    disponible.notify_all();
    for (auto& hilo : trabajadores) hilo.join();
}

EjecutorParalelo::~EjecutorParalelo() { detener(); }

void EjecutorParalelo::ejecutar(std::size_t cantidad,
                              const std::function<void(std::size_t)>& tarea) {
    if (trabajadores.empty()) {
        for (std::size_t i = 0; i < cantidad; ++i) tarea(i);
        return;
    }
    std::unique_lock<std::mutex> bloqueo(mutex);
    tareaActual = tarea;
    cantidadActual = cantidad;
    siguiente = 0;
    error = nullptr;
    pendientes = trabajadores.size();
    ++ronda;
    disponible.notify_all();
    terminado.wait(bloqueo, [this] { return pendientes == 0; });
    tareaActual = {};
    if (error) std::rethrow_exception(error);
}

void EjecutorParalelo::trabajar() {
    std::size_t vista = 0;
    for (;;) {
        std::unique_lock<std::mutex> bloqueo(mutex);
        disponible.wait(bloqueo, [&] { return cerrando || ronda != vista; });
        if (cerrando) return;
        vista = ronda;
        bloqueo.unlock();
        for (;;) {
            const auto indice = siguiente.fetch_add(1, std::memory_order_relaxed);
            if (indice >= cantidadActual) break;
            try {
                tareaActual(indice);
            } catch (...) {
                std::lock_guard<std::mutex> guardar(mutex);
                if (!error) error = std::current_exception();
            }
        }
        bloqueo.lock();
        if (--pendientes == 0) terminado.notify_one();
    }
}
