import random
import math
import time

archivo = "e18.stp"
tiempo_limite = 600   # segundos que corre el recocido

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


# ---------- funciones ----------
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
        return None   # no quedo conexo

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
    import heapq
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


# ---------- 2. solucion base: MST sobre todo el grafo ----------
base = mst_y_poda(set(range(1, num_nodos + 1)))
# (esto ya incluye la poda, el MST sin podar se calcula aparte)
padre = list(range(num_nodos + 1))

def buscar2(x):
    while padre[x] != x:
        padre[x] = padre[padre[x]]
        x = padre[x]
    return x

costo_mst = 0
for u, v, w in aristas_ordenadas:
    ru = buscar2(u)
    rv = buscar2(v)
    if ru != rv:
        padre[ru] = rv
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

# ---------- 4. mejora: recocido simulado sobre los nodos de Steiner ----------
temp_inicial = 0.3
temp_final = 0.08

actual = set(mejor_nodos)
costo_actual = mejor_costo
mejor = set(actual)
costo_mejor = costo_actual

inicio = time.time()
iteracion = 0
while time.time() - inicio < tiempo_limite:
    iteracion += 1
    avance = (time.time() - inicio) / tiempo_limite
    temperatura = temp_inicial * (temp_final / temp_inicial) ** avance

    nuevo = set(actual)
    steiner = []
    for n in actual:
        if n not in es_terminal:
            steiner.append(n)
    if len(steiner) == 0:
        continue

    # vecino al azar de un nodo del arbol (candidato a agregar)
    a = random.choice(list(actual))
    candidato = random.choice(grafo[a])[0]

    jugada = random.random()
    if jugada < 0.3:
        # intercambio: se saca un steiner y se mete un vecino del arbol
        if candidato in actual:
            continue
        nuevo.discard(random.choice(steiner))
        nuevo.add(candidato)
    elif jugada < 0.65:
        # quitar un nodo de steiner
        nuevo.discard(random.choice(steiner))
    else:
        # agregar un vecino del arbol
        if candidato in actual:
            continue
        nuevo.add(candidato)

    r = mst_y_poda(nuevo)
    if r is None:
        continue
    diferencia = r[0] - costo_actual
    if diferencia <= 0 or random.random() < math.exp(-diferencia / temperatura):
        actual = set(r[1].keys())
        costo_actual = r[0]
        if costo_actual < costo_mejor:
            costo_mejor = costo_actual
            mejor = set(actual)

    if iteracion % 10000 == 0:
        print("iteracion", iteracion, "mejor costo:", costo_mejor)

# ---------- 5. validacion ----------
resultado = mst_y_poda(mejor)
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