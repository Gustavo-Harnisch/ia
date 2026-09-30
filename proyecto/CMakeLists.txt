cmake_minimum_required(VERSION 3.16)
project(Steiner LANGUAGES CXX)

find_package(Threads REQUIRED)

find_program(GRAPHVIZ_DOT NAMES dot)
if(NOT GRAPHVIZ_DOT)
    message(FATAL_ERROR "Se necesita Graphviz (comando dot) para generar las imágenes.")
endif()

# Cada nuevo archivo .cpp del programa debe agregarse a esta lista.
add_executable(proyecto
    src/main.cpp
    src/Grafo.cpp
    src/Lectura.cpp
    src/Grafica.cpp
    src/MST.cpp
    src/Steiner.cpp
    src/Heuristica.cpp
    src/BusquedaLocal.cpp
    src/Evolutivo.cpp
)
target_link_libraries(proyecto PRIVATE Threads::Threads)
target_compile_definitions(proyecto PRIVATE
    PROYECTO_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    GRAPHVIZ_DOT="${GRAPHVIZ_DOT}"
)
set_target_properties(proyecto PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(proyecto PRIVATE -Wall -Wextra -Wpedantic)
endif()
