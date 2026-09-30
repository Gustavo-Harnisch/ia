# Datos de entrada

`grafo.txt` contiene el ejemplo de seis nodos. Puedes editarlo sin cambiar el código C++.

Su formato de aprendizaje es:

```text
NODOS 6
TERMINALES 3
0 3 5
ARISTAS 6
0 1 4
1 2 2
2 3 3
1 4 1
4 3 5
4 5 2
```

- `NODOS`: cantidad total. Los identificadores van desde 0 hasta cantidad menos 1.
- `TERMINALES`: cantidad de nodos obligatorios, seguida de sus identificadores sin repetir.
- `ARISTAS`: cantidad de conexiones, seguida de una línea `origen destino costo` por conexión.
- Las conexiones no tienen dirección: se escribe cada una una sola vez.
- Usar costos enteros no negativos y conexiones entre nodos diferentes.
- Actualizar las cantidades cuando se agreguen o eliminen datos. El lector no admite comentarios.

Para ejecutar este ejemplo, cambia `archivoPredeterminado` a `"grafo.txt"`
en `src/main.cpp` y pulsa **▶ Run Code** después de editar el TXT.
Las imágenes se generan en `salidas/grafo.png` y `salidas/grafo.svg`.

Este formato sencillo **no es SteinLib**.

## Instancia de SteinLib descargada

`e18.stp` se extrajo sin modificar del [conjunto E oficial](https://steinlib.zib.de/download/E.tgz).
Se comprobaron sus 2.500 nodos, 62.500 aristas y 417 terminales: coinciden con los datos del PDF.
Esta coincidencia identifica E18 como la candidata al ejercicio; todavía no se ha comparado
con el archivo adjunto del profesor.

El programa ahora ejecuta `e18.stp` por defecto. `Lectura.cpp` detecta automáticamente
el TXT de aprendizaje o SteinLib STP 1.0 no dirigido. El lector admite las secciones
`Comment`, `Graph` y `Terminals`; rechaza otras variantes en vez de interpretarlas
como si fueran el mismo problema.

- `Nodes 2500`: cantidad de nodos.
- `Edges 62500`: cantidad de conexiones.
- `E 387 666 9`: conexión entre los nodos 387 y 666 con costo 9.
- `Terminals 417`: cantidad de terminales.
- `T 2200`: el nodo 2200 es obligatorio.

Los identificadores en SteinLib comienzan en 1. El lector resta 1 para guardarlos
en el grafo: el nodo original 2200 se guarda internamente como 2199. Para reportar
una solución con los identificadores originales habrá que sumar 1 nuevamente.

Se verifican las cantidades declaradas, los límites de los identificadores, los
terminales repetidos, los costos y el cierre de las secciones y del archivo.
Los costos admitidos son enteros no negativos.

El resumen de E18 debe mostrar 2.500 nodos, 62.500 aristas y 417 terminales.
La conectividad se comprueba sobre todos sus datos. Solo se dibujan automáticamente
grafos de hasta 80 nodos y 200 aristas; E18 no genera imagen por su tamaño.

También se puede pasar un archivo como argumento, desde la carpeta del proyecto:

```bash
./build/proyecto datos/e18.stp
./build/proyecto datos/grafo.txt
```
