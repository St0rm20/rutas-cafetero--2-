#!/usr/bin/env bash
# Borra datos procesados y resultados generados (deja el catalogo y el codigo).
cd "$(dirname "$0")/.."
rm -f data/raw/*.graphml data/processed/* results/*.csv results/*.json results/*.txt results/*.html results/*.geojson results/figuras/*.png
echo "Carpetas data/raw, data/processed y results limpias."
