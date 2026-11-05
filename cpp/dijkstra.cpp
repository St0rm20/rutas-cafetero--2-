// dijkstra.cpp - Requerimiento 2: caminos minimos (implementacion propia)
//
// Dos versiones de Dijkstra sobre listas de adyacencia (CSR):
//   lineal : busca el minimo con un recorrido de todos los nodos, O(V^2 + E)
//   pq     : cola de prioridad (std::priority_queue con eliminacion perezosa), O((V+E) log V)
// Ambas se detienen al sacar el nodo destino (si hay destino) y reconstruyen la ruta con
// el predecesor de cada nodo. Cada nodo cuenta como explorado una sola vez, la primera
// vez que sale de la cola.
//
// Modos:
//   dijkstra bench  <dir> <pares.csv> <salida.csv> [limite_s]
//   dijkstra routes <dir> <pares_nodos.txt>            (imprime rutas por la salida estandar)
//   dijkstra matrix <dir> <lugares_nodos.csv> <salida.csv>
#include <queue>
#include <limits>
#include "common.hpp"

static const double INF = std::numeric_limits<double>::infinity();

struct Result {
    double dist = INF;       // tiempo de viaje al destino (s)
    ll explored = 0;         // nodos explorados
    bool ok = true;          // false si se paso el limite de tiempo
};

// ---- Version 1: minimo por recorrido lineal ----
// Invariante: al sacar u con la menor distancia entre los nodos no fijados, dist[u] ya es
// la distancia minima real (los pesos son >= 0).
Result dijkstra_lineal(const Graph& g, int s, int t, std::vector<double>& dist,
                       std::vector<int>* pred, double limite_s) {
    int V = g.V;
    dist.assign(V, INF);
    std::vector<char> fijo(V, 0);
    if (pred) pred->assign(V, -1);
    dist[s] = 0.0;
    Result r;
    auto t0 = Clock::now();
    for (int it = 0; it < V; ++it) {
        int u = -1;
        double best = INF;
        for (int i = 0; i < V; ++i)                // recorrido lineal: O(V) por extraccion
            if (!fijo[i] && dist[i] < best) { best = dist[i]; u = i; }
        if (u == -1) break;                         // lo que queda es inalcanzable
        fijo[u] = 1;
        r.explored++;
        if (u == t) break;                          // parada temprana
        for (int e = g.off[u]; e < g.off[u + 1]; ++e) {
            int v = g.to[e];
            double nd = dist[u] + g.w[e];
            if (!fijo[v] && nd < dist[v]) { dist[v] = nd; if (pred) (*pred)[v] = u; }
        }
        if ((it & 255) == 0 && ms_desde(t0) > limite_s * 1000.0) { r.ok = false; return r; }
    }
    r.dist = (t >= 0) ? dist[t] : 0.0;
    return r;
}

// ---- Version 2: cola de prioridad ----
Result dijkstra_pq(const Graph& g, int s, int t, std::vector<double>& dist,
                   std::vector<int>* pred, double limite_s) {
    int V = g.V;
    dist.assign(V, INF);
    std::vector<char> fijo(V, 0);
    if (pred) pred->assign(V, -1);
    using Item = std::pair<double, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;
    dist[s] = 0.0;
    pq.push({0.0, s});
    Result r;
    auto t0 = Clock::now();
    ll iter = 0;
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (fijo[u]) continue;                      // entrada vieja: el nodo ya salio antes
        fijo[u] = 1;
        r.explored++;
        if (u == t) break;
        for (int e = g.off[u]; e < g.off[u + 1]; ++e) {
            int v = g.to[e];
            double nd = d + g.w[e];
            if (!fijo[v] && nd < dist[v]) {
                dist[v] = nd;
                if (pred) (*pred)[v] = u;
                pq.push({nd, v});
            }
        }
        if ((++iter & 1023) == 0 && ms_desde(t0) > limite_s * 1000.0) { r.ok = false; return r; }
    }
    r.dist = (t >= 0) ? dist[t] : 0.0;
    return r;
}

static std::vector<int> reconstruir(const std::vector<int>& pred, int s, int t) {
    std::vector<int> ruta;
    for (int v = t; v != -1; v = pred[v]) {
        ruta.push_back(v);
        if (v == s) break;
    }
    std::reverse(ruta.begin(), ruta.end());
    return ruta;
}

// ------------------------------------------------------------------------------------
static int modo_bench(const std::string& dir, const std::string& pares, const std::string& salida,
                      double limite_s) {
    Graph g = cargar_grafo(dir);
    std::ifstream f(pares);
    if (!f) { std::cerr << "No se pudo abrir " << pares << "\n"; return 1; }
    std::ofstream out(salida);
    out << "origen,destino,version,tiempo_viaje_s,nodos_explorados,tiempo_ms,estado\n";
    out.setf(std::ios::fixed);
    out.precision(4);
    std::string line;
    std::getline(f, line);  // encabezado: origen,destino,nodo_origen,nodo_destino,lineal
    int discrepancias = 0;
    std::vector<double> dist;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto c = split(line);
        int s = std::atoi(c[2].c_str()), t = std::atoi(c[3].c_str());
        bool con_lineal = std::atoi(c[4].c_str()) == 1;
        double viaje_pq = -1, viaje_lin = -1;
        for (int v = 0; v < 2; ++v) {
            bool lineal = (v == 0);
            if (lineal && !con_lineal) continue;
            Result res;
            double ms = 0;
            auto corrida = [&]() {
                res = lineal ? dijkstra_lineal(g, s, t, dist, nullptr, limite_s)
                             : dijkstra_pq(g, s, t, dist, nullptr, limite_s);
                return res.ok;
            };
            bool ok = medir_mediana(corrida, ms);
            const char* ver = lineal ? "lineal" : "cola_prioridad";
            if (ok) {
                out << c[0] << ',' << c[1] << ',' << ver << ',' << res.dist << ',' << res.explored << ','
                    << ms << ",ok\n";
                (lineal ? viaje_lin : viaje_pq) = res.dist;
            } else {
                out << c[0] << ',' << c[1] << ',' << ver << ",,,,no_completada\n";
            }
            std::cerr << c[0] << "->" << c[1] << " " << ver << " " << (ok ? ms : -1) << " ms\n";
        }
        if (con_lineal && viaje_lin >= 0 && viaje_pq >= 0 && std::abs(viaje_lin - viaje_pq) > 1e-6) {
            std::cerr << "DISCREPANCIA en " << c[0] << "->" << c[1] << ": " << viaje_lin << " vs " << viaje_pq << "\n";
            discrepancias++;
        }
    }
    if (discrepancias) { std::cerr << discrepancias << " discrepancias entre versiones\n"; return 2; }
    std::cerr << "Verificacion: ambas versiones coinciden en todos los pares medidos con las dos.\n";
    return 0;
}

static int modo_routes(const std::string& dir, const std::string& archivo) {
    Graph g = cargar_grafo(dir);
    std::ifstream f(archivo);
    if (!f) { std::cerr << "No se pudo abrir " << archivo << "\n"; return 1; }
    std::cout << "origen,destino,tiempo_s,explorados,ruta\n";
    std::vector<double> dist;
    std::vector<int> pred;
    int s, t;
    while (f >> s >> t) {
        Result r = dijkstra_pq(g, s, t, dist, &pred, 1e9);
        std::cout << s << ',' << t << ',' << r.dist << ',' << r.explored << ',';
        if (r.dist < INF) {
            auto ruta = reconstruir(pred, s, t);
            for (size_t i = 0; i < ruta.size(); ++i) std::cout << (i ? " " : "") << ruta[i];
        }
        std::cout << "\n";
    }
    return 0;
}

static int modo_matrix(const std::string& dir, const std::string& lugares, const std::string& salida) {
    Graph g = cargar_grafo(dir);
    std::ifstream f(lugares);
    if (!f) { std::cerr << "No se pudo abrir " << lugares << "\n"; return 1; }
    std::vector<std::string> ids;
    std::vector<int> nodos;
    std::string line;
    std::getline(f, line);  // id,nodo
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto c = split(line);
        ids.push_back(c[0]);
        nodos.push_back(std::atoi(c[1].c_str()));
    }
    int n = (int)ids.size();
    std::ofstream out(salida);
    out.setf(std::ios::fixed);
    out.precision(3);
    out << "id";
    for (auto& id : ids) out << ',' << id;
    out << "\n";
    std::vector<double> dist;
    auto t0 = Clock::now();
    for (int i = 0; i < n; ++i) {
        // una ejecucion completa (sin destino) desde cada lugar
        dijkstra_pq(g, nodos[i], -1, dist, nullptr, 1e9);
        out << ids[i];
        for (int j = 0; j < n; ++j) out << ',' << dist[nodos[j]];
        out << "\n";
    }
    std::cerr << "Matriz " << n << "x" << n << " calculada en " << ms_desde(t0) << " ms\n";
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uso:\n  dijkstra bench <dir> <pares.csv> <salida.csv> [limite_s]\n"
                     "  dijkstra routes <dir> <pares_nodos.txt>\n"
                     "  dijkstra matrix <dir> <lugares_nodos.csv> <salida.csv>\n";
        return 1;
    }
    std::string modo = argv[1];
    if (modo == "bench" && argc >= 5) return modo_bench(argv[2], argv[3], argv[4], argc >= 6 ? std::atof(argv[5]) : 300.0);
    if (modo == "routes" && argc >= 4) return modo_routes(argv[2], argv[3]);
    if (modo == "matrix" && argc >= 5) return modo_matrix(argv[2], argv[3], argv[4]);
    std::cerr << "Argumentos invalidos\n";
    return 1;
}
