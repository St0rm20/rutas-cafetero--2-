# Guía paso a paso: Proyecto final «Rutas del Paisaje Cultural Cafetero»

Esta guía recorre el proyecto en el orden en que conviene hacerlo. Cada paso dice **qué ejecutar**, **qué debe salir**, **qué verificar** y **qué va en el documento técnico**.

## 0. Mapa del repositorio

```
cpp/        dijkstra.cpp, hull.cpp, tsp.cpp, common.hpp     (algoritmos propios, C++17)
python/     r1_*.py ... r4_*.py, viajante.py, common.py      (datos, experimentos, gráficas)
app/        app.py + templates/index.html                    (R5, Flask + Leaflet)
specs/      una especificación por requerimiento             (Spec-Driven Development)
tests/      test_dijkstra.py, test_tsp.py, test_hull.py      (verificación de cada algoritmo)
data/       catalogo.csv (borrador), raw/ (descargas), processed/ (grafo procesado)
results/    CSV, JSON y figuras que alimentan el documento
docs/       documento_tecnico_base.md (teoría ya redactada)
scripts/    compilar.sh, ejecutar_todo.sh, prueba_sintetica.sh, limpiar.sh
```

Flujo de datos: `catalogo.csv` + red OSM → **R1** → `nodes.csv`, `edges.csv`, `lugares_nodos.csv` → **R2** (C++ `dijkstra`) → `matriz_tiempos.csv` → **R4** (C++ `tsp`). **R3** usa solo `nodes.csv`. **R5** lee todo lo anterior.

## 1. Preparar el entorno

```bash
# Fedora:  sudo dnf install gcc-c++ cmake python3-pip
# Debian/Ubuntu: sudo apt install g++ cmake python3-pip python3-venv
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
bash scripts/compilar.sh          # genera build/dijkstra, build/hull, build/tsp
```

**Prueba rápida del código (2 min, sin internet):** `bash scripts/prueba_sintetica.sh` ejecuta todo sobre una red inventada. Sirve para comprobar que su entorno funciona. Sus resultados **no** son los del proyecto: al terminar ejecute `bash scripts/limpiar.sh`.

**Git desde el primer día:** el historial de commits es evidencia del trabajo de cada integrante. Haga commits pequeños y que cada integrante suba los suyos.

## 2. Catálogo (antes de R1)

`data/catalogo.csv` es un **borrador**: 12 cabeceras, 15 atractivos, 8 fincas/hoteles y 5 centros de acopio. Las coordenadas son aproximadas, de memoria, y varias están seguramente desplazadas.

1. `python python/r1_verificar_catalogo.py` y abra `results/catalogo_mapa.html`.
2. Corrija cada coordenada con Google Maps / OSM (clic derecho → copiar coordenadas). Las cabeceras van en la **plaza principal**; Armenia es la plaza de Bolívar.
3. Reemplace las fincas y centros de acopio por lugares reales que el grupo escoja (el enunciado dice «a elección del grupo»).
4. Revise `duracion_visita_min` de los 15 atractivos y escriba en `fuente_duracion` la fuente o el criterio (por ejemplo, la duración sugerida en la página oficial del sitio). Esto se cita en el documento.
5. Evite comas dentro de los campos (los programas en C++ leen CSV simple).

## 3. R1 — Modelo de la red vial (15 %)

```bash
python python/r1_preparar.py     # descarga (varios minutos), pesos, componente, lugares, exportación
python python/r1_tiempos.py      # descarga radios 2/5/10/20 km y mide largest_component
```

Si la descarga falla, pida el GraphML al docente y use `python python/r1_preparar.py --graphml archivo.graphml`.

| Punto | Dónde sale | Qué escribir en el documento |
|---|---|---|
| Representación | `results/r1_memoria.csv` | V y E reales, supuestos (4 u 8 B por celda; 12 o 24 B por arista) y la elección. Con V² celdas la matriz es del orden de GB y las listas del orden de MB, y la red es dispersa (E ≈ 3V); se elige listas de adyacencia |
| Pesos | `results/r1_metadatos.json` | Cuántas aristas traían `maxspeed`, cuántas se imputaron por tipo de vía, la tabla de velocidades y la velocidad de respaldo. Aclarar que el tiempo es una **estimación del modelo** |
| Componente | `results/r1_algoritmo.txt` | Algoritmo que usa la versión instalada (ver abajo), su descripción y complejidad |
| Tiempos | `results/r1_tiempos.csv` y figura | Tabla del anexo y comparación con O(V+E): si `us_por_elemento` es casi constante, coincide |
| Lugares lejanos | `results/r1_lugares_lejanos.csv` | Lista y tratamiento (ver abajo) |
| Tamaño | `results/r1_tamano.csv` | Nodos y aristas antes y después |

**Qué algoritmo usa `largest_component`.** En la versión que se probó (OSMnx 2.1.1, NetworkX 3.7) la función primero llama `nx.is_strongly_connected` y, si el grafo no es fuertemente conexo, calcula `nx.strongly_connected_components` (versión iterativa del algoritmo de Tarjan con las mejoras de Tarjan y Zwick, 2024), toma la de mayor tamaño y construye un nuevo `MultiDiGraph` con ese subgrafo. Es O(V + E) en tiempo y O(V) en memoria extra. **Confirme con su versión**: `results/r1_algoritmo.txt` guarda el código fuente y el docstring que se usaron.

**Por qué conviene la componente fuertemente conexa:** dentro de ella existe camino dirigido entre cualquier par de nodos. Así Dijkstra nunca devuelve «inalcanzable», la matriz de tiempos queda sin infinitos y el viajante siempre tiene solución.

**Lugares a más de 500 m.** Revise primero si la coordenada está mal y corríjala. Si el lugar es legítimamente rural, justifique la decisión (se conserva con su nodo más cercano y se deja constancia de que el tramo en línea recta no se suma al tiempo, o se reemplaza el lugar). Escoja una y documéntela.

**Si se cae por RAM** con el departamento completo: cierre otras aplicaciones; la red de Quindío es pequeña (decenas de miles de nodos) y no debería ser problema.

## 4. R2 — Caminos mínimos (20 %)

```bash
python python/r2_experimentos.py
```

Hace todo: elige 20 pares con semilla fija (10 cercanos medidos con las dos versiones), llama a `build/dijkstra bench`, genera `results/r2_mediciones.csv` (formato del anexo), la gráfica, el cociente, la verificación y la matriz de tiempos.

Verificación (queda en `results/r2_verificacion.txt` y se explica en el documento): (a) las dos versiones dan el mismo tiempo en los 10 pares comunes; (b) 10 pares contra NetworkX; (c) `tests/test_dijkstra.py` con un grafo resuelto a mano y 25 grafos aleatorios.

Para el documento (30 % teórico):
- **Invariante:** cuando un nodo u sale de la cola con la menor distancia tentativa, `dist[u]` ya es la distancia mínima real desde el origen. Todo nodo fijado tiene distancia final; todo nodo no fijado tiene `dist` igual al mejor camino que pasa solo por nodos fijados.
- **Por qué se puede parar al sacar el destino:** en ese momento su distancia ya es definitiva y los nodos restantes tienen distancia tentativa mayor o igual; ningún camino futuro puede mejorarla.
- **Condición sobre los pesos:** no negativos. Con un peso negativo, un nodo ya fijado podría mejorarse después y el invariante falla. Aquí los tiempos son estrictamente positivos.
- **Complejidades:** lineal O(V² + E) = O(V²); cola de prioridad O((V + E) log V). La implementación usa `std::priority_queue` con eliminación perezosa: hay hasta E entradas, así que cada operación cuesta O(log E) = O(log V²) = O(log V). Memoria O(V + E).
- **Por qué el cociente crece con el tamaño de la red:** la versión lineal recorre los V nodos en cada extracción aunque solo se exploren unos pocos, por eso el costo depende del tamaño de toda la red y no solo de la zona explorada.

## 5. R3 — Envolvente convexa (20 %)

```bash
python tests/test_hull.py          # verificación en casos pequeños
python python/r3_experimentos.py   # tabla, GeoJSON y figura
```

**Algoritmo elegido:** divide y vencerás por mezcla de envolventes. Se ordena una vez, se parte en dos mitades, se resuelve cada una y se combinan recorriendo los vértices de ambas envolventes con una sola pasada de cadena monotona. Recurrencia `T(n) = 2T(n/2) + O(n)` → O(n log n) en el peor caso; memoria O(n).

**Justificación según la distribución:** los nodos de una red vial están muy concentrados en las ciudades y la envolvente tiene pocos vértices (h ≪ n), pero con tantos puntos de distribución irregular un algoritmo cuyo costo depende de cómo se reparten los puntos (Quickhull, cuyo peor caso es O(n²)) puede degradarse. Con la mezcla el costo está acotado en O(n log n) sin importar la distribución, y el costo de combinar es proporcional a h, que es pequeño. (Si el docente espera Quickhull, puede cambiarlo: la interfaz de `hull.cpp` no cambia.)

**SciPy:** `scipy.spatial.ConvexHull` envuelve la biblioteca **Qhull**, escrita en C, que implementa el algoritmo **Quickhull** (Barber, Dobkin y Huhdanpaa, 1996). Cite la documentación de SciPy y ese artículo. Lo esperable es que SciPy gane por factor constante (C optimizado, sin ordenar todo, trabaja con punto flotante); la tabla del anexo lleva la columna «Cociente». Explique el resultado que obtenga, no el que espere.

## 6. R4 — Ruta óptima de recorrido (25 %)

```bash
python python/r4_experimentos.py     # tarda unos minutos: la fuerza bruta con k = 12 hace 479 millones de recorridos
python python/r4_itinerario.py       # itinerario de un día
```

- **Soluciones exactas:** `results/r4_exactas.csv` y `figuras/r4_tiempo_vs_k.png`. Reporte el mayor k resuelto por cada método. Si una ejecución pasa de 5 min el programa la marca «no completada».
- **Subestructura óptima (justificar antes de implementar):** si el mejor recorrido que sale del inicio, visita el conjunto S y termina en j pasa por i justo antes de j, entonces su tramo hasta i debe ser el mejor camino que visita S \ {j} y termina en i; si no, se podría reemplazarlo por uno mejor y mejorar el total. Recurrencia en `specs/r4_viajante.md`. Tiempo O(2^k · k²), memoria O(2^k · k) (la columna `memoria_hk_mib` ya la calcula).
- **Heurística elegida: vecino más cercano.** Justificación: es la voraz más natural, O(k²), no necesita estructura adicional. Si el grupo prefiere otra (inserción más barata, por ejemplo), cambie `vecino_mas_cercano()` en `tsp.cpp`.
- **Calidad:** `results/r4_calidad.csv` (brecha promedio y máxima, 5 instancias por k).
- **Itinerario:** `results/r4_itinerario.csv` con el formato del anexo y `r4_itinerario_resumen.json` con totales. Defina los atractivos con `--atractivos T01,T03,...` (6 a 8). Si no caben en 11 h, el script quita el que más tiempo consume (visita + desvío) y recalcula; justifíquelo en el texto.
- **Voraz frente a óptimo (punto 5):** use `results/r4_ejemplo_voraz.txt` y el ejemplo mínimo de abajo. **Dijkstra sí es óptimo con la elección voraz** porque tiene el invariante de arriba: elegir el nodo de menor distancia tentativa es seguro, pues con pesos no negativos ningún desvío posterior puede acortarlo. En el viajante una elección local (ir al más cercano) compromete el resto del recorrido y no existe una propiedad de elección voraz que garantice el óptimo.

Ejemplo mínimo a mano (4 lugares, matriz asimétrica; la fila es el origen y la columna el destino, partida 0):

| | 0 | 1 | 2 | 3 |
|---|---|---|---|---|
| **0** | 0 | 3 | 2 | 8 |
| **1** | 4 | 0 | 2 | 1 |
| **2** | 3 | 7 | 0 | 4 |
| **3** | 8 | 4 | 3 | 0 |

El vecino más cercano sale al 2 (cuesta 2, el menor desde 0), luego al 3 (4, pues 7 > 4) y luego al 1 (4); regresa con 4: total **14**. El recorrido 0→1→3→2→0 cuesta 3 + 1 + 3 + 3 = **10**. La elección más barata al inicio (ir al 2) obliga después a pagar tramos caros; esa es la razón por la que la elección voraz no garantiza el óptimo aquí. Este mismo caso sirve como prueba en `tests/test_tsp.py`.

## 7. R5 — Aplicación (5 %)

```bash
python app/app.py       # http://127.0.0.1:5000
```

Pestañas: Ruta, Envolvente, Itinerario (Held-Karp en azul, voraz en naranja punteada, con diferencia) y Resultados. Necesita internet para los mosaicos del mapa. Docker es opcional.

## 8. Documento técnico

`docs/documento_tecnico_base.md` trae redactadas la arquitectura, la teoría de cada requerimiento (problema, complejidad con fuente, recurrencias, correctitud) y las conclusiones, con marcas «[COMPLETAR]» donde van sus números. Está escrito en forma impersonal, con las reglas de puntuación que indicó. Quede pendiente: pegar tablas y figuras de `results/`, la máquina de `results/maquina.json`, la fecha de descarga (`r1_metadatos.json`) y el anexo de IA (plantilla incluida).

## 9. Lista final antes de entregar

- [ ] Catálogo con coordenadas verificadas y fuentes de duración.
- [ ] `scripts/limpiar.sh` ejecutado **antes** de correr con la red real (sin restos sintéticos).
- [ ] Todos los `results/*.csv` y figuras generados con la red real, en la misma máquina.
- [ ] Los tests pasan (`tests/*.py`).
- [ ] README probado desde cero en otra carpeta.
- [ ] Commits de todos los integrantes; repositorio compartido con el docente.
- [ ] Anexo de IA completo, y cada integrante puede explicar cualquier parte del código.

## Referencias sugeridas (verifique cada una antes de citarla)

- Cormen, Leiserson, Rivest y Stein. *Introduction to Algorithms* (4.ª ed.), 2022. Dijkstra (cap. 22), componentes fuertemente conexas, envolvente convexa (cap. 33).
- Tarjan, R. (1972). Depth-first search and linear graph algorithms. *SIAM Journal on Computing*, 1(2).
- Tarjan, R. y Zwick, U. (2024). Finding strong components using depth-first search. *European Journal of Combinatorics*, 119. https://doi.org/10.1016/j.ejc.2023.103815
- Boeing, G. (2017). OSMnx: New methods for acquiring, constructing, analyzing, and visualizing complex street networks. *Computers, Environment and Urban Systems*, 65.
- Held, M. y Karp, R. (1962). A dynamic programming approach to sequencing problems. *Journal of the SIAM*, 10(1).
- Barber, C. B., Dobkin, D. P. y Huhdanpaa, H. (1996). The Quickhull algorithm for convex hulls. *ACM Transactions on Mathematical Software*, 22(4).
- Preparata, F. y Hong, S. (1977). Convex hulls of finite sets of points in two and three dimensions. *Communications of the ACM*, 20(2).
- Documentación de SciPy: `scipy.spatial.ConvexHull`; documentación de NetworkX y OSMnx.
