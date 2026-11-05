"""Verifica fuerza bruta, Held-Karp y la heuristica voraz con matrices pequenas (algunas resueltas a mano).
Ejecutar:  python tests/test_tsp.py
"""
import random
import sys
import tempfile
from pathlib import Path

import numpy as np
import pandas as pd

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "python"))
from viajante import costo_orden, resolver  # noqa: E402


def escribir_matriz(M, ruta):
    n = len(M)
    ids = [f"L{i}" for i in range(n)]
    pd.DataFrame(M, index=ids, columns=ids).rename_axis("id").to_csv(ruta)


def main():
    rnd = random.Random(1)
    with tempfile.TemporaryDirectory() as d:
        ruta = Path(d) / "m.csv"
        # 1) caso resuelto a mano (asimetrico): el optimo es 0->1->2->0 con costo 1+1+1 = 3
        M = [[0, 1, 9, 9], [9, 0, 1, 9], [1, 9, 0, 9], [9, 9, 9, 0]]
        escribir_matriz(M, ruta)
        r = resolver([("a", 0, [1, 2])], ("brute", "hk", "greedy"), matriz=ruta)
        assert all(abs(r.costo_s - 3) < 1e-9), r
        assert r[r.algoritmo == "hk"].iloc[0].orden == [0, 1, 2, 0]

        # 1b) ejemplo de la guia: el vecino mas cercano da 14 y el optimo es 10
        M2 = [[0, 3, 2, 8], [4, 0, 2, 1], [3, 7, 0, 4], [8, 4, 3, 0]]
        escribir_matriz(M2, ruta)
        r = resolver([("g", 0, [1, 2, 3])], ("brute", "hk", "greedy"), matriz=ruta).set_index("algoritmo")
        assert r.loc["hk", "costo_s"] == 10 and r.loc["brute", "costo_s"] == 10 and r.loc["greedy", "costo_s"] == 14
        assert r.loc["hk", "orden"] == [0, 1, 3, 2, 0] and r.loc["greedy", "orden"] == [0, 2, 3, 1, 0]
        escribir_matriz(M, ruta)

        # 2) k = 1: ida y vuelta
        r = resolver([("b", 0, [2])], ("brute", "hk", "greedy"), matriz=ruta)
        assert all(abs(r.costo_s - (M[0][2] + M[2][0])) < 1e-9)

        # 3) matrices asimetricas aleatorias: brute == hk, el orden suma el costo, el voraz no mejora el optimo
        for t in range(40):
            n = rnd.randint(4, 9)
            M = np.array([[0 if i == j else rnd.randint(1, 50) for j in range(n)] for i in range(n)], dtype=float)
            escribir_matriz(M, ruta)
            k = rnd.randint(2, n - 1)
            lug = rnd.sample(range(1, n), k)
            r = resolver([("x", 0, lug)], ("brute", "hk", "greedy"), matriz=ruta).set_index("algoritmo")
            mat = pd.read_csv(ruta, index_col=0)
            assert abs(r.loc["brute", "costo_s"] - r.loc["hk", "costo_s"]) < 1e-6
            assert abs(costo_orden(mat, r.loc["hk", "orden"]) - r.loc["hk", "costo_s"]) < 1e-6
            assert abs(costo_orden(mat, r.loc["greedy", "orden"]) - r.loc["greedy", "costo_s"]) < 1e-6
            assert r.loc["greedy", "costo_s"] >= r.loc["hk", "costo_s"] - 1e-6
            assert sorted(r.loc["hk", "orden"][1:-1]) == sorted(lug)
    print("OK: caso a mano, k=1 y 40 matrices asimetricas aleatorias")


if __name__ == "__main__":
    main()
