"""Genera una red SINTETICA con el mismo formato que devuelve OSMnx (MultiDiGraph guardado en GraphML).

Sirve solo para probar el codigo cuando no hay conexion a OpenStreetMap. Los resultados que salgan
de esta red NO son los del proyecto: para el informe se debe usar la red real descargada.

Uso: python python/sintetico.py [--paso 700] [--salida data/raw/sintetico.graphml]
"""
import argparse
import random

import networkx as nx
import numpy as np
import osmnx as ox

from common import RAW, SEMILLA, a_geo, crear_carpetas


def construir(paso=700.0, semilla=SEMILLA):
    rng = random.Random(semilla)
    # caja aproximada del Quindio en UTM 18N (metros)
    x0, x1, y0, y1 = 393000.0, 451000.0, 440000.0, 532000.0
    nx_, ny_ = int((x1 - x0) / paso), int((y1 - y0) / paso)
    G = nx.MultiDiGraph(crs="epsg:4326")
    ids = {}
    nid = 1
    xs, ys = [], []
    for i in range(nx_):
        for j in range(ny_):
            x = x0 + i * paso + rng.uniform(-120, 120)
            y = y0 + j * paso + rng.uniform(-120, 120)
            ids[(i, j)] = nid
            xs.append(x); ys.append(y)
            nid += 1
    lat, lon = a_geo(np.array(xs), np.array(ys))
    for (i, j), n in ids.items():
        G.add_node(n, x=float(lon[n - 1]), y=float(lat[n - 1]), _ux=xs[n - 1], _uy=ys[n - 1])
    tipos = [("primary", "60"), ("secondary", "50"), ("tertiary", None), ("residential", None), ("unclassified", None)]
    pesos = [0.08, 0.17, 0.30, 0.35, 0.10]

    def agregar(a, b):
        ax, ay = G.nodes[a]["_ux"], G.nodes[a]["_uy"]
        bx, by = G.nodes[b]["_ux"], G.nodes[b]["_uy"]
        largo = float(np.hypot(ax - bx, ay - by)) * rng.uniform(1.0, 1.4)
        tipo, vel = rng.choices(tipos, pesos)[0]
        atr = dict(length=largo, highway=tipo)
        if vel and rng.random() < 0.7:
            atr["maxspeed"] = vel
        sentido = rng.random()
        if sentido < 0.85:       # doble sentido
            G.add_edge(a, b, **atr); G.add_edge(b, a, **atr)
        elif sentido < 0.93:
            G.add_edge(a, b, **atr)
        else:
            G.add_edge(b, a, **atr)

    for (i, j), n in ids.items():
        for di, dj in ((1, 0), (0, 1)):
            m = ids.get((i + di, j + dj))
            if m is not None and rng.random() < 0.93:   # se pierden algunas vias (red irregular)
                agregar(n, m)
    for n in G.nodes:
        del G.nodes[n]["_ux"], G.nodes[n]["_uy"]
    return G


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--paso", type=float, default=700.0)
    ap.add_argument("--salida", default=str(RAW / "sintetico.graphml"))
    a = ap.parse_args()
    crear_carpetas()
    G = construir(a.paso)
    ox.save_graphml(G, a.salida)
    print(f"Red sintetica: {G.number_of_nodes()} nodos, {G.number_of_edges()} aristas -> {a.salida}")
