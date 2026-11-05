#!/usr/bin/env bash
# Pipeline completo sobre la red REAL (necesita internet para la descarga de OpenStreetMap).
set -e
cd "$(dirname "$0")/.."
bash scripts/compilar.sh
python3 python/r1_verificar_catalogo.py          # revise results/catalogo_mapa.html y corrija data/catalogo.csv
python3 python/r1_preparar.py ${1:+--graphml "$1"}
python3 python/r1_tiempos.py
python3 python/r2_experimentos.py
python3 tests/test_dijkstra.py
python3 tests/test_tsp.py
python3 tests/test_hull.py
python3 python/r3_experimentos.py
python3 python/r4_experimentos.py
python3 python/r4_itinerario.py
echo "Listo. Para la aplicacion: python3 app/app.py"
