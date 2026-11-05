# Rutas del Paisaje Cultural Cafetero: documento técnico (borrador base)

*Marcas [COMPLETAR]: valores que salen de `results/` al ejecutar el proyecto con la red real. Los párrafos están en forma impersonal y alternan `;` y `.` como terminadores, según las reglas de redacción indicadas. Las tablas y las listas quedan fuera de esa regla.*

Integrantes: [COMPLETAR] · Curso: [COMPLETAR] · Fecha: [COMPLETAR]

## 1. Arquitectura de la solución

La solución se organiza en cinco componentes que se comunican mediante archivos CSV; de esta manera, cada requerimiento puede ejecutarse y verificarse por separado.

```
 OpenStreetMap ──► R1 preparación (Python, OSMnx) ──► nodes.csv, edges.csv, lugares_nodos.csv
 catalogo.csv  ──►                                            │
                                                              ├──► R2 dijkstra (C++) ──► matriz_tiempos.csv ──► R4 tsp (C++)
                                                              └──► R3 hull (C++)  ──► envolvente.geojson
 R1..R4 ──► results/*.csv, figuras ──► R5 aplicación (Flask + Leaflet; llama a dijkstra y tsp)
```

Los algoritmos propios (Dijkstra, envolvente convexa, fuerza bruta, Held-Karp y vecino más cercano) están escritos en C++ para que todas las mediciones sean comparables; la preparación de datos, las gráficas y la aplicación se programaron en Python, ya que OSMnx es una librería de Python.

El grafo procesado se exporta a dos archivos (nodos y aristas con sus pesos) que los programas en C++ leen y almacenan en listas de adyacencia compactas; a partir de esto, las gráficas y las tablas del documento se generan siempre desde archivos de resultados, sin escribir valores a mano.

Cada requerimiento se especificó antes de programarse mediante desarrollo guiado por especificaciones (una descripción breve del comportamiento esperado y de los casos de prueba, guardada en la carpeta `specs/`); lo anterior permitió verificar el código contra algo escrito de antemano, y no solo contra lo que el programa devolvía.

**Condiciones experimentales:** Todas las mediciones de un mismo requerimiento se hicieron en la misma máquina: procesador [COMPLETAR], memoria RAM [COMPLETAR] GB, sistema operativo [COMPLETAR], compilador [COMPLETAR] con la bandera `-O2` y Python [COMPLETAR]. Cada medición se repitió cinco veces y se reporta la mediana (las ejecuciones de más de un minuto se midieron una sola vez); los datos aleatorios usan la semilla 42, y el límite de tiempo por ejecución fue de cinco minutos.

## 2. Requerimiento 1. Modelo de la red vial

El requerimiento convierte la red vial real del Quindío en un grafo dirigido (un conjunto de nodos unidos por aristas que tienen un sentido de circulación) sobre el cual puedan ejecutarse los demás algoritmos. Los nodos son intersecciones y extremos de vías, y las aristas son tramos de vía con su longitud y, en ocasiones, su velocidad máxima; la red se descargó con OSMnx (Boeing, 2017) el [COMPLETAR fecha] y se obtuvieron [COMPLETAR] nodos y [COMPLETAR] aristas.

**Representación:** Una matriz de adyacencia (una tabla de V filas y V columnas donde cada celda guarda el peso de la arista entre dos nodos) requiere V² celdas; con 4 bytes por celda ocuparía [COMPLETAR] MiB y con 8 bytes [COMPLETAR] MiB. Las listas de adyacencia (para cada nodo, solo la lista de las aristas que salen de él) guardan únicamente las E aristas existentes; con 12 bytes por arista (destino, tiempo y distancia) ocuparían [COMPLETAR] MiB y con 24 bytes [COMPLETAR] MiB.

Una red vial es dispersa, es decir, cada intersección se conecta con muy pocas vías (E es del orden de tres veces V) y casi todas las celdas de la matriz quedarían vacías; por esta razón se eligieron las listas de adyacencia, almacenadas de forma compacta en tres arreglos (desplazamientos, destinos y pesos).

**Pesos:** Cada arista guarda su distancia en metros y su tiempo de viaje en segundos, calculado como distancia dividida entre la velocidad del tramo. La velocidad se tomó de la etiqueta `maxspeed` de OpenStreetMap cuando existe ([COMPLETAR] aristas) y, cuando no, se asignó una velocidad según el tipo de vía (por ejemplo 60 km/h para vías primarias y 30 km/h para residenciales), con 30 km/h como valor de respaldo para tipos no previstos; el tiempo obtenido es una estimación del modelo y no el tiempo real de recorrido, ya que no considera tráfico, semáforos ni el estado de la vía.

**Componente fuertemente conexa:** Una componente fuertemente conexa es un conjunto de nodos en el que, respetando el sentido de las vías, se puede ir de cualquiera a cualquier otro. Para obtener la más grande se usó `ox.truncate.largest_component(G, strongly=True)`; en la versión instalada ([COMPLETAR versión], ver `results/r1_algoritmo.txt`) la función comprueba primero si el grafo ya es fuertemente conexo con `nx.is_strongly_connected` y, de no serlo, calcula las componentes con `nx.strongly_connected_components` y construye un nuevo grafo con la de mayor tamaño.

El algoritmo de NetworkX es una versión iterativa (sin recursión) del algoritmo de Tarjan con las mejoras descritas por Tarjan y Zwick (2024); funciona con una búsqueda en profundidad que asigna a cada nodo un número de descubrimiento y el menor número alcanzable desde él, y cuando ambos coinciden el nodo es la raíz de una componente, que se extrae de una pila. Cada nodo y cada arista se recorren un número constante de veces, por lo que el tiempo es O(V + E) y la memoria adicional es O(V) (Tarjan, 1972; Cormen, Leiserson, Rivest y Stein, 2022).

La medición con redes de radio 2, 5, 10 y 20 km alrededor de la plaza de Bolívar y con el departamento completo se presenta en la Tabla R1-3 y en la Figura R1-1; [COMPLETAR: si el tiempo por elemento (V + E) se mantiene casi constante, lo medido coincide con O(V + E), y si crece, explicar por qué, por ejemplo por la copia del subgrafo o por la memoria caché].

Trabajar solo con esta componente conviene porque dentro de ella existe un camino dirigido entre cualquier par de nodos; de esta manera, Dijkstra nunca devuelve «inalcanzable», la matriz de tiempos queda sin valores infinitos y el problema del viajante siempre tiene solución.

**Ubicación de los lugares:** Cada lugar del catálogo se asoció con el nodo de la componente más cercano en línea recta, usando la distancia euclidiana entre coordenadas en metros (proyección UTM zona 18N) y un árbol k-d (estructura que organiza puntos en el plano para encontrar el vecino más cercano en tiempo logarítmico promedio). [COMPLETAR: N] lugares quedaron a más de 500 m de su nodo (Tabla R1-4); [COMPLETAR: tratamiento dado a cada uno].

**Tablas y figuras de R1** (pegar desde `results/`): Tabla R1-1 `r1_tamano.csv` · Tabla R1-2 `r1_memoria.csv` · Tabla R1-3 `r1_tiempos.csv` · Tabla R1-4 `r1_lugares_lejanos.csv` · Figura R1-1 `r1_tiempos.png`.

## 3. Requerimiento 2. Caminos mínimos

El problema consiste en hallar el recorrido de menor tiempo entre dos lugares sobre la componente de R1; para ello se implementó el algoritmo de Dijkstra en dos versiones, una que busca el nodo de menor distancia con un recorrido lineal y otra que usa una cola de prioridad (una estructura que entrega siempre el elemento de menor valor). La versión lineal tiene complejidad O(V²) en tiempo, porque en cada una de las V extracciones recorre los V nodos; la versión con cola tiene O((V + E) log V), ya que cada arista puede generar una inserción de costo logarítmico (Cormen, Leiserson, Rivest y Stein, 2022).

En memoria ambas usan O(V + E), pues guardan el grafo, un arreglo de distancias, uno de predecesores y uno de nodos fijados; la cola de prioridad implementada permite entradas repetidas (eliminación perezosa), por lo que puede contener hasta E elementos, y esto no cambia el orden O(log V) de cada operación, porque log E es menor o igual que 2 log V.

**Correctitud:** El invariante de Dijkstra establece que, cuando un nodo sale de la cola con la menor distancia tentativa, esa distancia es la mínima real desde el origen. Además, para todo nodo no fijado, la distancia tentativa es el costo del mejor camino que usa solo nodos ya fijados; por esta razón es posible detenerse al sacar el destino: su distancia ya es definitiva y cualquier otro camino debería pasar por un nodo aún no fijado, cuya distancia es mayor o igual.

El invariante exige que los pesos no sean negativos. Con un peso negativo, un nodo ya fijado podría mejorarse después y la demostración dejaría de ser válida; en este proyecto todos los tiempos de viaje son estrictamente positivos, pues se calculan como longitud sobre velocidad, y por lo tanto se cumple la condición.

**Verificación:** Las dos versiones se compararon en los 10 pares medidos con ambas y arrojaron los mismos tiempos de viaje (diferencia máxima [COMPLETAR] s); además, diez pares se contrastaron con `nx.dijkstra_path_length` (diferencia máxima [COMPLETAR] s), y se ejecutaron pruebas automáticas con un grafo resuelto a mano y con grafos aleatorios (`tests/test_dijkstra.py`).

**Experimento:** Se midieron 20 pares origen-destino con la cola de prioridad y 10 de ellos, entre lugares cercanos, con la versión lineal (Figura R2-1). La mediana del cociente entre el tiempo de la versión lineal y el de la cola fue [COMPLETAR], con mínimo [COMPLETAR] y máximo [COMPLETAR] (Tabla R2-1); [COMPLETAR: la versión lineal recorre todos los nodos en cada extracción aunque solo explore una parte de la red, por eso el cociente crece con el tamaño del grafo, en coherencia con O(V²) frente a O((V + E) log V)].

**Matriz de tiempos:** Se calculó una matriz de [COMPLETAR] × [COMPLETAR] con una ejecución completa de Dijkstra (con cola, sin detenerse en un destino) desde cada lugar; la matriz no es simétrica, ya que el sentido de las vías hace que ir de un lugar a otro pueda tardar distinto que regresar, y su mayor diferencia entre t(i, j) y t(j, i) fue de [COMPLETAR] s.

## 4. Requerimiento 3. Envolvente convexa de la red vial

La envolvente convexa de un conjunto de puntos es el menor polígono convexo que los contiene. Calculada sobre los nodos de la componente, da una aproximación de la extensión de la red vial; las coordenadas se convirtieron a metros (UTM zona 18N), se aproximaron al entero más cercano y se eliminaron los puntos repetidos, lo que permite calcular giros y alineaciones con enteros de 64 bits sin errores de redondeo.

**Algoritmo elegido:** Se implementó un algoritmo de divide y vencerás por mezcla de envolventes: los puntos se ordenan una vez por coordenada, se dividen en dos mitades, se calcula recursivamente la envolvente de cada una y se combinan recorriendo los vértices de ambas con una pasada de cadena monótona (que descarta, mediante el signo del producto cruz, los puntos que no hacen un giro hacia la izquierda); como todos los puntos de la mitad izquierda quedan antes que los de la derecha, la lista de vértices de ambas envolventes ya está ordenada y no hace falta ordenar de nuevo.

La combinación es correcta porque todo vértice de la envolvente de la unión es también vértice de la envolvente de una de las dos partes. En consecuencia, basta con examinar los vértices de las dos envolventes y no todos los puntos; los puntos alineados sobre un lado no se cuentan como vértices, ya que la cadena monótona descarta los giros nulos.

**Complejidad:** La recurrencia es T(n) = 2T(n/2) + O(n), pues combinar cuesta a lo sumo proporcional al número de vértices de las dos mitades, que no supera n; por el teorema maestro, el costo es O(n log n) en el peor caso y la memoria es O(n), incluido el ordenamiento inicial, que también cuesta O(n log n) (Preparata y Hong, 1977, y Cormen, Leiserson, Rivest y Stein, 2022).

**Justificación según la distribución:** Los nodos de una red vial se concentran en las ciudades y siguen trazados irregulares, y la envolvente suele tener muy pocos vértices frente al total de puntos. Con esta distribución el costo de combinar es pequeño (proporcional a esos pocos vértices), y el algoritmo conserva el límite O(n log n) sin depender de cómo se reparten los puntos; en cambio, Quickhull (Barber, Dobkin y Huhdanpaa, 1996) tiene peor caso O(n²) cuando las particiones quedan desbalanceadas, situación que una red con zonas muy densas podría provocar.

**Verificación:** La envolvente propia se comparó con `scipy.spatial.ConvexHull` en los cuatro conjuntos medidos y en 300 conjuntos pequeños aleatorios con muchos puntos repetidos y alineados (`tests/test_hull.py`), y los vértices coincidieron en todos los casos; además, se contrastó con una cadena monótona independiente escrita en Python.

**Experimento:** La Tabla R3-1 presenta los tiempos con los nodos de la red ([COMPLETAR] puntos distintos, [COMPLETAR] vértices) y con puntos al azar en un cuadrado (1 000, 10 000 y 50 000, semilla 42); [COMPLETAR: SciPy envuelve la biblioteca Qhull, escrita en C, que implementa el algoritmo Quickhull, por esta razón, y por ejecutar código compilado y optimizado sobre decimales sin ordenar todos los puntos, se espera que supere a la implementación propia por un factor constante, y se discute el cociente obtenido].

## 5. Requerimiento 4. Ruta óptima de recorrido

El problema consiste en salir de un lugar, visitar k lugares más y regresar al punto de partida con el menor tiempo total. Como ir de i a j puede tardar distinto que ir de j a i, se trata del problema del viajante asimétrico; es un problema difícil, en el sentido de que no se conoce un algoritmo exacto de costo polinomial, por lo que las soluciones exactas crecen de forma exponencial con k y obligan a elegir entre exactitud y tiempo de cómputo.

**Fuerza bruta:** Se recorren todas las permutaciones de los k lugares (sin podas) y se conserva la de menor costo; hay k! permutaciones y cada una cuesta O(k) si el costo se acumula durante la búsqueda, de modo que el tiempo es O(k · k!) y la memoria O(k) por la profundidad de la recursión.

**Subestructura óptima:** Si el mejor recorrido que sale del inicio, visita el conjunto S y termina en j pasa por i inmediatamente antes de j, entonces su tramo hasta i debe ser el mejor recorrido que visita S sin j y termina en i; de lo contrario, se podría reemplazar ese tramo por uno más barato y mejorar el total, lo cual contradice que el recorrido sea óptimo. Esta propiedad permite plantear la recurrencia de Held-Karp (Held y Karp, 1962), donde dp[S][j] es el menor costo de salir del inicio, visitar exactamente los lugares de S y terminar en j:

```
dp[{j}][j] = c[0][j]
dp[S][j]   = min sobre i en S \ {j} de  dp[S \ {j}][i] + c[i][j]
óptimo     = min sobre j de  dp[todos][j] + c[j][0]
```

La tabla tiene 2^k · k entradas y cada una se calcula examinando hasta k predecesores, por lo que el tiempo es O(2^k · k²) y la memoria O(2^k · k). Con 8 bytes por costo y 1 byte por predecesor, la tabla ocupa 9 · 2^k · k bytes (por ejemplo [COMPLETAR] MiB para k = 12); el recorrido se reconstruye siguiendo la tabla de predecesores desde el mejor estado final.

**Verificación:** Fuerza bruta y Held-Karp entregaron el mismo costo óptimo en todos los k resueltos por ambas; además, el orden que devuelve Held-Karp suma exactamente el costo informado y visita cada lugar una vez, y se probaron matrices asimétricas aleatorias y un caso resuelto a mano (`tests/test_tsp.py`).

**Experimento de soluciones exactas:** Con salida desde Armenia y k lugares elegidos al azar (semilla 42), se aumentó k de 5 a 12 (Tabla R4-1 y Figura R4-1). El mayor k resuelto fue [COMPLETAR] con fuerza bruta y [COMPLETAR] con Held-Karp; [COMPLETAR: comparar el crecimiento factorial con el exponencial 2^k, y explicar que la ventaja de Held-Karp proviene de reutilizar los subproblemas, a costa de memoria].

**Heurística voraz:** Se eligió el vecino más cercano: desde el lugar actual se viaja siempre al lugar no visitado de menor tiempo, y al final se regresa al inicio; su costo es O(k²) en tiempo y O(k) en memoria. Se escogió por su simplicidad y porque es la estrategia voraz más natural para este problema, además de no requerir estructuras adicionales; su calidad se midió con 5 instancias para k = 5, 8, 10 y 12 (Tabla R4-2), con una brecha promedio de [COMPLETAR] % y máxima de [COMPLETAR] %.

**Itinerario de un día:** Con salida desde [COMPLETAR lugar] y [COMPLETAR] atractivos, Held-Karp entregó el orden de la Tabla R4-3. La duración de las visitas más los desplazamientos fue de [COMPLETAR] h frente a una jornada de 11 h; [COMPLETAR: si no cabía, se quitó en cada paso el atractivo que más tiempo consumía (duración de su visita más el desvío que causaba), con el fin de conservar la mayor cantidad posible de atractivos, y el recorrido final duró [COMPLETAR] h].

**Voraz frente a óptimo:** La heurística no siempre encuentra el mejor recorrido porque su decisión local compromete el resto del trayecto. En el ejemplo de cuatro lugares de la guía del proyecto, el vecino más cercano cuesta 14 y el óptimo cuesta 10; con datos del Quindío, el caso de mayor brecha ([COMPLETAR] %) se muestra en `results/r4_ejemplo_voraz.txt`.

Dijkstra también es voraz, pero allí la elección sí garantiza el óptimo: tomar el nodo de menor distancia tentativa es seguro porque, con pesos no negativos, ningún camino que pase por un nodo aún no fijado puede resultar más corto. En el viajante no existe una propiedad equivalente, pues ir al lugar más cercano puede obligar a pagar tramos largos más adelante; es decir, la voracidad es correcta cuando el problema tiene esa propiedad de elección segura y falla cuando no la tiene.

## 6. Requerimiento 5. Aplicación

La aplicación es una página web servida localmente con Flask que muestra los resultados del proyecto sobre un mapa de Leaflet; las rutas y los itinerarios se calculan en el momento llamando a los programas en C++ (Dijkstra con cola de prioridad, Held-Karp y vecino más cercano), mientras que la envolvente convexa se lee del archivo ya calculado. Permite elegir origen y destino y ver la ruta con su tiempo, ver la envolvente, elegir una partida y entre 6 y 8 atractivos para comparar los recorridos de Held-Karp y de la heurística voraz con su diferencia, y consultar las tablas y gráficas de R1 a R4; las instrucciones de ejecución están en el README del repositorio.

## 7. Conclusiones

[COMPLETAR con los resultados reales; estructura sugerida, en forma impersonal]

A partir de las mediciones realizadas se concluye que [COMPLETAR: la representación con listas de adyacencia fue la adecuada, pues la matriz habría requerido X MiB frente a Y MiB], y que el tiempo de obtener la componente fuertemente conexa [COMPLETAR: coincide o no con O(V + E)]; esto confirma que la complejidad teórica es un buen predictor del comportamiento con datos reales, aunque con diferencias en los factores constantes.

En los caminos mínimos se infiere que la versión con cola de prioridad es [COMPLETAR] veces más rápida que la lineal (mediana), y que esa ventaja aumenta con el tamaño de la red; en la envolvente convexa, se observa que la implementación propia [COMPLETAR] frente a SciPy, lo cual se explica por [COMPLETAR].

En el problema del viajante se concluye que Held-Karp resolvió hasta k = [COMPLETAR] frente a k = [COMPLETAR] de la fuerza bruta, y que la heurística del vecino más cercano se alejó del óptimo en promedio [COMPLETAR] %. Por esta razón, en un servicio que deba responder rápido con muchos lugares sería preferible la heurística, mientras que para pocos lugares la solución exacta es viable; con respecto al cumplimiento de los objetivos, se considera que se logró modelar el problema como grafo, diseñar experimentos reproducibles y comparar estrategias en calidad y en tiempo.

## 8. Anexo. Uso de inteligencia artificial

Se utilizó [COMPLETAR: herramienta y versión, por ejemplo Claude] como apoyo en [COMPLETAR: generación de un borrador del código en C++ y Python, de las especificaciones y de la estructura del documento]; las respuestas se verificaron mediante pruebas automáticas contra casos resueltos a mano, contra NetworkX y SciPy, y mediante la revisión del código línea por línea por parte de cada integrante.

Las coordenadas del catálogo propuestas por la herramienta se trataron como un borrador y se verificaron en un mapa. Las duraciones de visita y las fuentes se definieron por el grupo; cada integrante puede explicar y modificar cualquier parte del código en la sustentación.

## Referencias

Barber, C. B., Dobkin, D. P. y Huhdanpaa, H. (1996). The Quickhull algorithm for convex hulls. *ACM Transactions on Mathematical Software*, 22(4), 469-483.

Boeing, G. (2017). OSMnx: New methods for acquiring, constructing, analyzing, and visualizing complex street networks. *Computers, Environment and Urban Systems*, 65, 126-139.

Cormen, T., Leiserson, C., Rivest, R. y Stein, C. (2022). *Introduction to Algorithms* (4.ª ed.). MIT Press.

Held, M. y Karp, R. (1962). A dynamic programming approach to sequencing problems. *Journal of the Society for Industrial and Applied Mathematics*, 10(1), 196-210.

Preparata, F. y Hong, S. (1977). Convex hulls of finite sets of points in two and three dimensions. *Communications of the ACM*, 20(2), 87-93.

Tarjan, R. (1972). Depth-first search and linear graph algorithms. *SIAM Journal on Computing*, 1(2), 146-160.

Tarjan, R. y Zwick, U. (2024). Finding strong components using depth-first search. *European Journal of Combinatorics*, 119. https://doi.org/10.1016/j.ejc.2023.103815

Documentación de SciPy: `scipy.spatial.ConvexHull`. Documentación de NetworkX: `strongly_connected_components`.
