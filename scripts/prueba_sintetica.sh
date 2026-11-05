#!/usr/bin/env bash
# Prueba del codigo con una red SINTETICA (sin internet). Los resultados NO sirven para el informe.
# Ejecute scripts/limpiar.sh al terminar, antes de correr el pipeline real.
set -e
cd "$(dirname "$0")/.."
bash scripts/compilar.sh
python3 python/sintetico.py
python3 python/r1_preparar.py --graphml data/raw/sintetico.graphml
python3 python/r2_experimentos.py
python3 tests/test_dijkstra.py
python3 tests/test_tsp.py
python3 tests/test_hull.py
python3 python/r3_experimentos.py
python3 python/r4_experimentos.py --hk-hasta 16
python3 python/r4_itinerario.py
echo "Prueba sintetica OK. Ejecute scripts/limpiar.sh para borrar estos resultados de prueba."
