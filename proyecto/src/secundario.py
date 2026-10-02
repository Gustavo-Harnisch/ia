import random
import math
import time
import heapq
import numpy as np
from numba import njit

archivo = "e18.stp"
tiempo_limite = 600   # segundos que corre el recocido
semilla = 1

# ---------- 1. leer el grafo ----------
aristas = []
terminales = []
num_nodos = 0

f = open(archivo, "r")
for linea in f:
    partes = linea.split()
    if len(partes) == 0:
        continue
    if partes[0] == "Nodes":
        num_nodos = int(partes[1])
    elif partes[0] == "E":
        aristas.append((int(partes[1]), int(partes[2]), int(partes[3])))
    elif partes[0] == "T":
        terminales.append(int(partes[1]))
f.close()

print("nodos:", num_nodos)
print("aristas:", len(aristas))
print("terminales:", len(terminales))

grafo = {}
for i in range(1, num_nodos + 1):
    grafo[i] = []
for u, v, w in aristas:
    grafo[u].append((v, w))
    grafo[v].append((u, w))

es_terminal = set(terminales)
aristas_ordenadas = sorted(aristas, key=lambda a: a[2])

random.seed(semilla)


# ---------- funciones en python normal ----------
def mst_y_poda(nodos):
    # Kruskal solo con los nodos activos y despues poda de hojas no terminales
    padre = {}
    for n in nodos:
        padre[n] = n

    def buscar(x):
        while padre[x] != x:
            padre[x] = padre[padre[x]]
            x = padre[x]
        return x

    arbol_aristas = []
    for u, v, w in aristas_ordenadas:
        if u in nodos and v in nodos:
            ru = buscar(u)
            rv = buscar(v)
            if ru != rv:
                padre[ru] = rv
                arbol_aristas.append((u, v, w))
                if len(arbol_aristas) == len(nodos) - 1:
                    break

    if len(arbol_aristas) != len(nodos) - 1:
        return None

    arbol = {}
    for u, v, w in arbol_aristas:
        if u not in arbol:
            arbol[u] = {}
        if v not in arbol:
            arbol[v] = {}
        arbol[u][v] = w
        arbol[v][u] = w

    hojas = []
    for n in arbol:
        if len(arbol[n]) == 1 and n not in es_terminal:
            hojas.append(n)
    while len(hojas) > 0:
        n = hojas.pop()
        if n not in arbol:
            continue
        vecino = list(arbol[n].keys())[0]
        del arbol[vecino][n]
        del arbol[n]
        if len(arbol[vecino]) == 1 and vecino not in es_terminal:
            hojas.append(vecino)

    costo = 0
    for n in arbol:
        for v in arbol[n]:
            costo += arbol[n][v]
    return costo // 2, arbol


def camino_minimo_desde_arbol(inicio):
    # heuristica constructiva: se parte de un terminal y se agrega
    # siempre el terminal mas cercano al arbol por su camino mas corto
    en_arbol = set([inicio])
    dist = {}
    for n in grafo:
        dist[n] = 10**9
    anterior = {}
    cola = []

    def relajar(fuentes):
        for s in fuentes:
            dist[s] = 0
            heapq.heappush(cola, (0, s))
        while len(cola) > 0:
            d, u = heapq.heappop(cola)
            if d > dist[u]:
                continue
            for v, w in grafo[u]:
                if d + w < dist[v]:
                    dist[v] = d + w
                    anterior[v] = u
                    heapq.heappush(cola, (d + w, v))

    relajar([inicio])
    faltan = set(terminales) - en_arbol
    while len(faltan) > 0:
        t = min(faltan, key=lambda x: dist[x])
        nuevos = []
        x = t
        while x not in en_arbol:
            en_arbol.add(x)
            nuevos.append(x)
            x = anterior[x]
        faltan = faltan - en_arbol
        relajar(nuevos)
    return en_arbol


def validar(arbol):
    # DFS desde un terminal
    inicio = terminales[0]
    visitados = set([inicio])
    pila = [inicio]
    while len(pila) > 0:
        u = pila.pop()
        for v in arbol[u]:
            if v not in visitados:
                visitados.add(v)
                pila.append(v)
    num_aristas = 0
    for n in arbol:
        num_aristas += len(arbol[n])
    num_aristas = num_aristas // 2

    conexo = len(visitados) == len(arbol)
    es_arbol = num_aristas == len(arbol) - 1
    tiene_terminales = True
    for t in terminales:
        if t not in arbol:
            tiene_terminales = False
    return conexo, es_arbol, tiene_terminales


# ---------- funciones rapidas (numba): MST + poda y recocido ----------
@njit
def evaluar(activo, es_term, eu, ev, ew, n, padre, grado, xr, suma, salida, pila):
    # MST (Kruskal) solo con los nodos activos y poda de hojas no terminales
    cuenta = 0
    for i in range(1, n + 1):
        if activo[i]:
            padre[i] = i
            grado[i] = 0
            xr[i] = 0
            suma[i] = 0
            cuenta += 1
    usadas = 0
    costo = 0
    for e in range(len(eu)):
        if usadas >= cuenta - 1:
            break
        u = eu[e]
        v = ev[e]
        if activo[u] and activo[v]:
            a = u
            while padre[a] != a:
                padre[a] = padre[padre[a]]
                a = padre[a]
            b = v
            while padre[b] != b:
                padre[b] = padre[padre[b]]
                b = padre[b]
            if a != b:
                padre[a] = b
                usadas += 1
                costo += ew[e]
                grado[u] += 1
                grado[v] += 1
                xr[u] = xr[u] ^ v
                xr[v] = xr[v] ^ u
                suma[u] += ew[e]
                suma[v] += ew[e]
    if usadas != cuenta - 1:
        return -1
    for i in range(n + 1):
        salida[i] = activo[i]
    tope = 0
    for i in range(1, n + 1):
        if activo[i] and grado[i] == 1 and es_term[i] == 0:
            pila[tope] = i
            tope += 1
    while tope > 0:
        tope -= 1
        h = pila[tope]
        if salida[h] == 0:
            continue
        vecino = xr[h]
        costo -= suma[h]
        salida[h] = 0
        grado[vecino] -= 1
        xr[vecino] = xr[vecino] ^ h
        suma[vecino] -= suma[h]
        if grado[vecino] == 1 and es_term[vecino] == 0:
            pila[tope] = vecino
            tope += 1
    return costo


@njit
def bloque_recocido(actual, costo, mejor, costo_mejor, iteraciones, t_ini, t_fin, sem,
                    es_term, eu, ev, ew, n, ady_ini, ady, padre, grado, xr, suma, pila):
    np.random.seed(sem)
    nuevo = np.zeros(n + 1, dtype=np.int8)
    salida = np.zeros(n + 1, dtype=np.int8)
    lista = np.zeros(n + 1, dtype=np.int64)
    steiner = np.zeros(n + 1, dtype=np.int64)
    for it in range(iteraciones):
        temperatura = t_ini * (t_fin / t_ini) ** (it / iteraciones)
        cantidad = 0
        cant_steiner = 0
        for i in range(1, n + 1):
            if actual[i]:
                lista[cantidad] = i
                cantidad += 1
                if es_term[i] == 0:
                    steiner[cant_steiner] = i
                    cant_steiner += 1
        a = lista[np.random.randint(0, cantidad)]
        grado_a = ady_ini[a + 1] - ady_ini[a]
        candidato = ady[ady_ini[a] + np.random.randint(0, grado_a)]
        for i in range(n + 1):
            nuevo[i] = actual[i]
        jugada = np.random.random()
        if jugada < 0.3:
            if actual[candidato] == 1 or cant_steiner == 0:
                continue
            nuevo[steiner[np.random.randint(0, cant_steiner)]] = 0
            nuevo[candidato] = 1
        elif jugada < 0.65:
            if cant_steiner == 0:
                continue
            nuevo[steiner[np.random.randint(0, cant_steiner)]] = 0
        else:
            if actual[candidato] == 1:
                continue
            nuevo[candidato] = 1
        c = evaluar(nuevo, es_term, eu, ev, ew, n, padre, grado, xr, suma, salida, pila)
        if c < 0:
            continue
        diferencia = c - costo
        if diferencia <= 0 or np.random.random() < math.exp(-diferencia / temperatura):
            for i in range(n + 1):
                actual[i] = salida[i]
            costo = c
            if costo < costo_mejor:
                costo_mejor = costo
                for i in range(n + 1):
                    mejor[i] = actual[i]
    return costo, costo_mejor


# ---------- 2. solucion base: MST sobre todo el grafo ----------
base = mst_y_poda(set(range(1, num_nodos + 1)))
padre_uf = list(range(num_nodos + 1))

def buscar2(x):
    while padre_uf[x] != x:
        padre_uf[x] = padre_uf[padre_uf[x]]
        x = padre_uf[x]
    return x

costo_mst = 0
for u, v, w in aristas_ordenadas:
    ru = buscar2(u)
    rv = buscar2(v)
    if ru != rv:
        padre_uf[ru] = rv
        costo_mst += w
print("costo MST base:", costo_mst)
print("costo MST podado:", base[0])

# ---------- 3. solucion inicial (heuristica constructiva) ----------
print("generando solucion inicial...")
mejor_costo = 10**9
mejor_nodos = None
for i in range(30):
    nodos = camino_minimo_desde_arbol(random.choice(terminales))
    r = mst_y_poda(nodos)
    if r[0] < mejor_costo:
        mejor_costo = r[0]
        mejor_nodos = set(r[1].keys())
print("costo solucion inicial:", mejor_costo)

# ---------- 4. mejora: recocido simulado (numba) ----------
# se pasan los datos a arreglos de numpy
eu = np.array([a[0] for a in aristas_ordenadas], dtype=np.int64)
ev = np.array([a[1] for a in aristas_ordenadas], dtype=np.int64)
ew = np.array([a[2] for a in aristas_ordenadas], dtype=np.int64)
es_term = np.zeros(num_nodos + 1, dtype=np.int8)
for t in terminales:
    es_term[t] = 1

ady_ini = np.zeros(num_nodos + 2, dtype=np.int64)
for i in range(1, num_nodos + 1):
    ady_ini[i + 1] = ady_ini[i] + len(grafo[i])
ady = np.zeros(ady_ini[num_nodos + 1], dtype=np.int64)
for i in range(1, num_nodos + 1):
    k = ady_ini[i]
    for v, w in grafo[i]:
        ady[k] = v
        k += 1

padre = np.zeros(num_nodos + 1, dtype=np.int64)
grado = np.zeros(num_nodos + 1, dtype=np.int64)
xr = np.zeros(num_nodos + 1, dtype=np.int64)
suma = np.zeros(num_nodos + 1, dtype=np.int64)
pila = np.zeros(num_nodos + 1, dtype=np.int64)

actual = np.zeros(num_nodos + 1, dtype=np.int8)
for n in mejor_nodos:
    actual[n] = 1
mejor = actual.copy()
costo_actual = mejor_costo
costo_mejor = mejor_costo

temp_inicial = 0.3
temp_final = 0.07
iter_por_bloque = 200000

print("compilando y corriendo el recocido...")
inicio = time.time()
bloque = 0
while time.time() - inicio < tiempo_limite:
    bloque += 1
    avance1 = (time.time() - inicio) / tiempo_limite
    avance2 = min(1.0, avance1 + 0.03)
    t1 = temp_inicial * (temp_final / temp_inicial) ** avance1
    t2 = temp_inicial * (temp_final / temp_inicial) ** avance2
    costo_actual, costo_mejor = bloque_recocido(actual, costo_actual, mejor, costo_mejor,
                                                iter_por_bloque, t1, t2, semilla * 1000 + bloque,
                                                es_term, eu, ev, ew, num_nodos, ady_ini, ady,
                                                padre, grado, xr, suma, pila)
    print("bloque", bloque, "mejor costo:", costo_mejor, "tiempo:", round(time.time() - inicio))

mejor_set = set()
for i in range(1, num_nodos + 1):
    if mejor[i] == 1:
        mejor_set.add(i)

# ---------- 5. validacion ----------
resultado = mst_y_poda(mejor_set)
costo_final = resultado[0]
arbol_final = resultado[1]
conexo, es_arbol, tiene_terminales = validar(arbol_final)
print("conexo:", conexo)
print("es arbol:", es_arbol)
print("tiene todos los terminales:", tiene_terminales)

# ---------- 6. metricas ----------
print("costo final:", costo_final)
print("costo MST base:", costo_mst)
print("reduccion respecto al MST base:", round((costo_mst - costo_final) / costo_mst * 100, 2), "%")
print("reduccion respecto al MST podado:", round((base[0] - costo_final) / base[0] * 100, 2), "%")