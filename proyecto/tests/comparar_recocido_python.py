"""Contrasta la construcción C++ con funciones originales, sin ejecutar 600 s.

Uso: python3 tests/comparar_recocido_python.py /ruta/al/ejecutable/proyecto
No necesita NumPy/Numba: extrae únicamente las dos funciones Python comunes.
"""

import ast
import heapq
from pathlib import Path
import random
import subprocess
import sys
import tempfile


def funciones_originales(archivo):
    modulo = ast.parse(archivo.read_text())
    modulo.body = [n for n in modulo.body if isinstance(n, ast.FunctionDef)
                   and n.name in {"mst_y_poda", "camino_minimo_desde_arbol"}]
    entorno = {"heapq": heapq}
    exec(compile(modulo, str(archivo), "exec"), entorno)
    return entorno


def main():
    binario = str(Path(sys.argv[1]).resolve())
    proyecto = Path(__file__).resolve().parents[1]
    fuentes = [("principal.py", "--recocido-tiempo"),
               ("secundario.py", "--recocido-bloques")]
    azar = random.Random(73)
    with tempfile.TemporaryDirectory(prefix="recocido-equivalencia-") as temporal:
        for caso in range(40):
            n = azar.randint(4, 12)
            terminales = azar.sample(range(1, n + 1), 3)
            aristas = [(i, i + 1, azar.randrange(6)) for i in range(1, n)]
            aristas += [(*azar.sample(range(1, n + 1), 2), azar.randrange(6))
                        for _ in range(2 * n)]
            azar.shuffle(aristas)
            grafo = {i: [] for i in range(1, n + 1)}
            for u, v, w in aristas:
                grafo[u].append((v, w))
                grafo[v].append((u, w))
            archivo = Path(temporal) / "grafo.txt"
            archivo.write_text(f"NODOS {n}\nTERMINALES 3\n"
                               + " ".join(str(t - 1) for t in terminales)
                               + f"\nARISTAS {len(aristas)}\n"
                               + "\n".join(f"{u-1} {v-1} {w}" for u, v, w in aristas) + "\n")
            for fuente, modo in fuentes:
                ref = funciones_originales(proyecto / "src" / fuente)
                ref.update(grafo=grafo, terminales=terminales, es_terminal=set(terminales),
                           aristas_ordenadas=sorted(aristas, key=lambda a: a[2]))
                podado = ref["mst_y_poda"](set(grafo))[0]
                iniciales = {ref["mst_y_poda"](ref["camino_minimo_desde_arbol"](t))[0]
                             for t in terminales}
                salida = subprocess.check_output([binario, modo, str(archivo), "0", "1"], text=True)
                campos = dict(linea.split(": ", 1) for linea in salida.splitlines() if ": " in linea)
                assert int(campos["Costo MST podado"]) == podado, (caso, fuente, salida)
                assert int(campos["Costo solución inicial"]) in iniciales, (caso, fuente, iniciales, salida)
                assert campos["Costo final recocido simulado Steiner"] == campos["Costo solución inicial"]
                assert campos["Solución válida (conexa, árbol y todos los terminales)"] == "Sí"
    print("80 comparaciones con las funciones originales de Python correctas.")


if __name__ == "__main__":
    main()
