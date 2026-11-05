# Rutas del Paisaje Cultural Cafetero

Sistema de planificación de rutas sobre la red vial real del Quindío (OpenStreetMap): caminos mínimos, envolvente convexa, problema del viajante y una aplicación web demostrativa. Los algoritmos propios están en **C++17**; la descarga, los experimentos, las gráficas y la aplicación, en **Python**.

Guía paso a paso: [`GUIA.md`](GUIA.md) · Documento técnico (borrador): [`docs/documento_tecnico_base.md`](docs/documento_tecnico_base.md) · Especificaciones: [`specs/`](specs/)

## Requisitos

- Linux con `g++` (C++17), `cmake` ≥ 3.16 y Python ≥ 3.10.
- Conexión a internet para descargar la red de OpenStreetMap y los mosaicos del mapa de la aplicación.

```bash
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
```

## Cómo reproducir los resultados

```bash
bash scripts/compilar.sh                         # build/dijkstra, build/hull, build/tsp
python python/r1_verificar_catalogo.py           # revisar results/catalogo_mapa.html
python python/r1_preparar.py                     # R1: descarga, pesos, componente, lugares
python python/r1_tiempos.py                      # R1: tiempo de la componente (radios 2/5/10/20 km)
python python/r2_experimentos.py                 # R2: Dijkstra, gráfica, cociente, matriz de tiempos
python python/r3_experimentos.py                 # R3: envolvente convexa propia vs SciPy
python python/r4_experimentos.py                 # R4: fuerza bruta, Held-Karp, heurística
python python/r4_itinerario.py                   # R4: itinerario de un día (11 h)
```

O todo de una vez: `bash scripts/ejecutar_todo.sh` (si la descarga falla, `bash scripts/ejecutar_todo.sh archivo.graphml`).

Pruebas automáticas: `python tests/test_dijkstra.py && python tests/test_tsp.py && python tests/test_hull.py`.

Prueba sin internet con una red inventada (solo para comprobar el entorno; **no** usar esos resultados): `bash scripts/prueba_sintetica.sh` y luego `bash scripts/limpiar.sh`.

## Aplicación (R5)

```bash
python app/app.py        # abrir http://127.0.0.1:5000
```

Ruta entre dos lugares (Dijkstra con cola de prioridad), envolvente convexa, itinerario con Held-Karp y con la heurística voraz, y tablas y figuras de R1 a R4. Debe ejecutarse después de R1 a R4.

## Protocolo experimental

Mismas máquina y banderas (`-O2`) en cada requerimiento (`results/maquina.json`); 5 repeticiones y mediana (una sola si pasa de 60 s); límite de 5 minutos por ejecución; semilla fija 42; solo se mide el algoritmo. Los resultados se guardan en `results/*.csv` y las figuras en `results/figuras/`.

## Estructura

```
cpp/      dijkstra.cpp · hull.cpp · tsp.cpp · common.hpp
python/   r1_*.py … r4_*.py · viajante.py · common.py · sintetico.py
app/      app.py · templates/index.html
data/     catalogo.csv · raw/ · processed/
results/  CSV, JSON y figuras generados
specs/    una especificación por requerimiento
tests/    pruebas de cada algoritmo
```

## Datos

Red vial: OpenStreetMap vía OSMnx (fecha de descarga en `results/r1_metadatos.json`). Catálogo: `data/catalogo.csv` (≥ 40 lugares, coordenadas verificadas en mapa).

## Uso de inteligencia artificial

Ver el anexo del documento técnico.
