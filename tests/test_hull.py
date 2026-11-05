"""Verificacion de la envolvente convexa propia (C++) en casos pequenos y con muchos puntos alineados.

Compara contra (a) una cadena monotona independiente escrita aqui en Python y (b) scipy.spatial.ConvexHull
cuando el conjunto no es degenerado. Ejecutar:  python tests/test_hull.py
"""
import random
import sys
from pathlib import Path

import numpy as np
import pandas as pd
from scipy.spatial import ConvexHull

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "python"))
from common import RES, correr, crear_carpetas, exe  # noqa: E402
from r3_experimentos import sin_colineales  # noqa: E402


def cruz(o, a, b):
    return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])


def referencia(pts):
    """Cadena monotona de Andrew (implementacion independiente de la de C++)."""
    pts = sorted(set(pts))
    if len(pts) <= 2:
        return set(pts)
    inf, sup = [], []
    for p in pts:
        while len(inf) >= 2 and cruz(inf[-2], inf[-1], p) <= 0:
            inf.pop()
        inf.append(p)
    for p in reversed(pts):
        while len(sup) >= 2 and cruz(sup[-2], sup[-1], p) <= 0:
            sup.pop()
        sup.append(p)
    return set(inf[:-1] + sup[:-1])


def propio(pts):
    crear_carpetas()
    f, s = RES / "_t_pts.csv", RES / "_t_out.csv"
    pd.DataFrame(pts, columns=["x", "y"]).to_csv(f, index=False)
    correr([exe("hull"), f, s, 1])
    v = pd.read_csv(s)
    res = [tuple(r) for r in v[["x", "y"]].values.tolist()]
    f.unlink(); s.unlink()
    return res


def main():
    rnd = random.Random(7)
    casos = 0
    for _ in range(300):
        lado = rnd.choice([2, 4, 8, 30, 1000])          # lados pequenos => muchas colineales y repetidos
        n = rnd.randint(1, 60)
        pts = [(rnd.randint(0, lado), rnd.randint(0, lado)) for _ in range(n)]
        got = propio(pts)
        esperado = referencia(pts)
        assert set(got) == esperado, f"Difiere de la referencia: pts={pts}\n got={got}\n esp={esperado}"
        assert len(got) == len(set(got)), "vertices repetidos"
        # el poligono debe ser estrictamente convexo y antihorario
        if len(got) >= 3:
            for i in range(len(got)):
                assert cruz(got[i - 1], got[i], got[(i + 1) % len(got)]) > 0, f"no convexo/antihorario: {got}"
        # contra SciPy cuando no es degenerado
        if len(set(pts)) >= 3 and len(esperado) >= 3:
            arr = np.array(sorted(set(pts)), dtype=float)
            try:
                sc = set(sin_colineales(np.array(sorted(set(pts)))[ConvexHull(arr).vertices]))
                assert sc == set(got), f"Difiere de SciPy: {pts}"
            except Exception as e:  # conjuntos degenerados que Qhull rechaza
                if e.__class__.__name__ != "QhullError":
                    raise
        casos += 1
    # casos explicitos
    assert set(propio([(1, 1)])) == {(1, 1)}
    assert set(propio([(0, 0), (5, 5), (3, 3), (5, 5)])) == {(0, 0), (5, 5)}          # todos alineados
    assert set(propio([(0, 0), (2, 0), (4, 0), (4, 4), (0, 4), (2, 2)])) == {(0, 0), (4, 0), (4, 4), (0, 4)}
    print(f"OK: {casos} casos aleatorios + 3 casos explicitos")


if __name__ == "__main__":
    main()
