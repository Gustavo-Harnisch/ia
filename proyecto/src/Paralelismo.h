#ifndef PARALELISMO_H
#define PARALELISMO_H

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

// 0 elige automáticamente; nunca se crean más trabajadores que tareas útiles.
unsigned resolverHilos(unsigned solicitados, unsigned tareas);

// Ejecutor síncrono con trabajadores persistentes. Una sola llamada activa por
// instancia; cada tarea debe escribir únicamente en su propio resultado.
class EjecutorParalelo {
public:
    explicit EjecutorParalelo(unsigned hilos);
    ~EjecutorParalelo();
    EjecutorParalelo(const EjecutorParalelo&) = delete;
    EjecutorParalelo& operator=(const EjecutorParalelo&) = delete;
    // Espera todas las tareas antes de retornar o propagar una excepción.
    void ejecutar(std::size_t cantidad, const std::function<void(std::size_t)>& tarea);
private:
    void trabajar();
    void detener();
    std::vector<std::thread> trabajadores;
    std::mutex mutex;
    std::condition_variable disponible, terminado;
    std::function<void(std::size_t)> tareaActual;
    std::atomic<std::size_t> siguiente{0};
    std::size_t cantidadActual = 0, ronda = 0, pendientes = 0;
    bool cerrando = false;
    std::exception_ptr error;
};
#endif
