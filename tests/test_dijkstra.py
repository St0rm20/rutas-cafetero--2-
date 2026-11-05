"""Verifica Dijkstra (ambas versiones) en un grafo pequeno resuelto a mano y contra NetworkX en grafos aleatorios.
Ejecutar:  python tests/test_dijkstra.py
"""
import random
import sys
import tempfile
from pathlib import Path

import networkx as nx
import pandas as pd

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "python"))
from common import correr, exe  # noqa: E402


def escribir_grafo(d, n, aristas):
    pd.DataFrame({"idx": range(n)}).to_csv(Path(d) / "nodes.csv", index=False)
    pd.DataFrame([(u, v, 1.0, t) for u, v, t in aristas], columns=["u", "v", "length_m", "time_s"]).to_csv(Path(d) / "edges.csv", index=False)


def bench(d, pares):
    p = Path(d) / "pares.csv"
    pd.DataFrame([(f"P{a}", f"P{b}", a, b, 1) for a, b in pares],
                 columns=["origen", "destino", "nodo_origen", "nodo_destino", "lineal"]).to_csv(p, index=False)
    correr([exe("dijkstra"), "bench", d, p, Path(d) / "out.csv", 60])
    return pd.read_csv(Path(d) / "out.csv")


def main():
    with tempfile.TemporaryDirectory() as d:
        # a mano: 0->1 (4), 0->2 (1), 2->1 (2), 1->3 (1), 2->3 (5). Camino minimo 0->3 = 0-2-1-3 = 4
        escribir_grafo(d, 4, [(0, 1, 4), (0, 2, 1), (2, 1, 2), (1, 3, 1), (2, 3, 5)])
        r = bench(d, [(0, 3)])
        assert (abs(r["tiempo_viaje_s"] - 4) < 1e-9).all(), r
        out, _ = correr([exe("dijkstra"), "routes", d, _escribir(d, "0 3\n")])
        assert out.strip().splitlines()[1].endswith("0 2 1 3"), out
        # sentido de las vias: 3 no llega a 0
        out, _ = correr([exe("dijkstra"), "routes", d, _escribir(d, "3 0\n")])
        assert ",inf," in out or ",inf" in out, out

        # aleatorios contra NetworkX
        rnd = random.Random(3)
        for t in range(25):
            n = rnd.randint(5, 60)
            aristas = [(rnd.randrange(n), rnd.randrange(n), rnd.randint(1, 30)) for _ in range(rnd.randint(n, 4 * n))]
            escribir_grafo(d, n, aristas)
            G = nx.DiGraph()
            for u, v, w in aristas:
                if u != v and (not G.has_edge(u, v) or w < G[u][v]["w"]):
                    G.add_edge(u, v, w=w)
            G.add_nodes_from(range(n))
            pares = [(a, b) for a, b in [(rnd.randrange(n), rnd.randrange(n)) for _ in range(8)] if nx.has_path(G, a, b)]
            if not pares:
                continue
            res = bench(d, pares)
            for (a, b), (_, fila) in zip(pares, res[res.version == "cola_prioridad"].iterrows()):
                assert abs(fila.tiempo_viaje_s - nx.dijkstra_path_length(G, a, b, weight="w")) < 1e-6
    print("OK: grafo a mano, sentido de vias y 25 grafos aleatorios contra NetworkX")


def _escribir(d, texto):
    p = Path(d) / "q.txt"
    p.write_text(texto)
    return p


if __name__ == "__main__":
    main()
