// hull.cpp - Requerimiento 3: envolvente convexa por divide y venceras (implementacion propia)
//
// Algoritmo: divide y venceras por mezcla de envolventes.
//   1. Se ordenan los puntos por (x, y) y se eliminan repetidos (una sola vez).
//   2. Dividir: se parte el arreglo ordenado en dos mitades.
//   3. Vencer: se calcula recursivamente la envolvente de cada mitad.
//   4. Combinar: como todos los puntos de la mitad izquierda quedan antes que los de la
//      derecha, los vertices de ambas envolventes ya forman una lista ordenada; sobre ella
//      se hace una sola pasada de cadena monotona (producto cruz) y se obtiene la envolvente
//      de la union. El costo de combinar es proporcional al numero de vertices de las dos
//      envolventes (h1 + h2 <= n).
// Recurrencia: T(n) = 2T(n/2) + O(n)  ->  O(n log n) en el peor caso, sin importar la
// distribucion de los puntos. Memoria: O(n).
// Los productos cruz usan enteros de 64 bits; los puntos alineados sobre un lado no se
// cuentan como vertices (se descartan con cross <= 0 / cross >= 0).
//
// Uso: hull <puntos.csv> <vertices_salida.csv> [repeticiones]
//   puntos.csv: encabezado x,y y coordenadas enteras (metros, UTM 18N)
//   salida estandar: n_unicos,h,tiempo_ms
#include "common.hpp"

struct P {
    ll x, y;
    bool operator<(const P& o) const { return x != o.x ? x < o.x : y < o.y; }
    bool operator==(const P& o) const { return x == o.x && y == o.y; }
};

static inline ll cross(const P& o, const P& a, const P& b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Envolvente como dos cadenas ordenadas de izquierda a derecha.
struct Hull {
    std::vector<P> lower, upper;
};

// Une dos cadenas (ambas ordenadas) en una lista ordenada sin repetir los extremos.
static std::vector<P> vertices_ordenados(const Hull& h) {
    std::vector<P> out;
    out.reserve(h.lower.size() + h.upper.size());
    std::merge(h.lower.begin(), h.lower.end(), h.upper.begin(), h.upper.end(), std::back_inserter(out));
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

// Cadena monotona sobre puntos ya ordenados (uno solo recorrido, O(m)).
static Hull cadena_monotona(const std::vector<P>& pts) {
    Hull h;
    for (const P& p : pts) {
        while (h.lower.size() >= 2 && cross(h.lower[h.lower.size() - 2], h.lower.back(), p) <= 0) h.lower.pop_back();
        h.lower.push_back(p);
    }
    for (const P& p : pts) {
        while (h.upper.size() >= 2 && cross(h.upper[h.upper.size() - 2], h.upper.back(), p) >= 0) h.upper.pop_back();
        h.upper.push_back(p);
    }
    return h;
}

// Envolvente de pts[l, r) (ya ordenados y sin repetidos)
static Hull dyv(const std::vector<P>& pts, int l, int r) {
    if (r - l == 1) return Hull{{pts[l]}, {pts[l]}};
    int mid = (l + r) / 2;
    Hull A = dyv(pts, l, mid);
    Hull B = dyv(pts, mid, r);
    std::vector<P> uni = vertices_ordenados(A);
    std::vector<P> vb = vertices_ordenados(B);
    uni.insert(uni.end(), vb.begin(), vb.end());      // A < B en el orden lexicografico
    return cadena_monotona(uni);
}

// Resultado en sentido antihorario, empezando por el punto mas a la izquierda y abajo.
static std::vector<P> envolvente(std::vector<P> pts) {
    std::sort(pts.begin(), pts.end());
    pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
    if (pts.empty()) return {};
    Hull h = dyv(pts, 0, (int)pts.size());
    std::vector<P> res = h.lower;
    if (h.lower.size() > 1)
        for (int i = (int)h.upper.size() - 2; i >= 1; --i) res.push_back(h.upper[i]);
    return res;
}

int main(int argc, char** argv) {
    if (argc < 3) { std::cerr << "Uso: hull <puntos.csv> <vertices.csv> [reps]\n"; return 1; }
    int reps = argc >= 4 ? std::atoi(argv[3]) : 5;
    std::ifstream f(argv[1]);
    if (!f) { std::cerr << "No se pudo abrir " << argv[1] << "\n"; return 1; }
    std::vector<P> pts;
    std::string line;
    std::getline(f, line);
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto c = split(line);
        pts.push_back({std::atoll(c[0].c_str()), std::atoll(c[1].c_str())});
    }
    std::vector<P> res;
    double ms = 0;
    // Solo se mide el algoritmo (ordenar + dividir + combinar); la lectura queda por fuera
    medir_mediana([&]() { res = envolvente(pts); return true; }, ms, reps);
    std::vector<P> unicos = pts;
    std::sort(unicos.begin(), unicos.end());
    unicos.erase(std::unique(unicos.begin(), unicos.end()), unicos.end());
    std::ofstream out(argv[2]);
    out << "x,y\n";
    for (auto& p : res) out << p.x << ',' << p.y << "\n";
    std::cout << unicos.size() << ',' << res.size() << ',' << ms << "\n";
    return 0;
}
