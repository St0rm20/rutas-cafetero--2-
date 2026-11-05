// tsp.cpp - Requerimiento 4: ruta optima de recorrido (problema del viajante asimetrico)
//
// Algoritmos (implementacion propia):
//   brute  : fuerza bruta, recorre todas las permutaciones de los k lugares (sin podas)
//   hk     : Held-Karp, programacion dinamica sobre subconjuntos, O(2^k * k^2) tiempo y
//            O(2^k * k) memoria; devuelve el orden de visita
//   greedy : heuristica voraz del vecino mas cercano, O(k^2)
//
// Uso: tsp <matriz.csv> <instancias.csv> <salida.csv> <algoritmos> [limite_s]
//   matriz.csv     : primera fila y primera columna con los identificadores; celda (i,j) = segundos de i a j
//   instancias.csv : encabezado inst,k,inicio,lugares  (indices de la matriz; "lugares" separados por espacio)
//   algoritmos     : lista separada por comas, por ejemplo brute,hk,greedy
//   salida.csv     : inst,k,algoritmo,tiempo_ms,costo_s,estado,orden
#include <limits>
#include <cstdint>
#include "common.hpp"

static const double INF = std::numeric_limits<double>::infinity();

struct Sol {
    double costo = INF;
    std::vector<int> orden;  // indices locales 0..k (0 = inicio), incluye inicio al principio y al final
    bool ok = true;
};

// ---------------- fuerza bruta ----------------
struct Brute {
    const std::vector<std::vector<double>>& c;
    int k;
    double best = INF;
    std::vector<int> cur, bestOrd;
    std::vector<char> usado;
    ll hojas = 0;
    Clock::time_point t0;
    double limite_ms;
    bool timeout = false;

    Brute(const std::vector<std::vector<double>>& c_, int k_, double limite_s)
        : c(c_), k(k_), usado(k_ + 1, 0), t0(Clock::now()), limite_ms(limite_s * 1000.0) {}

    void dfs(int prof, int ultimo, double costo) {
        if (timeout) return;
        if (prof == k) {
            double total = costo + c[ultimo][0];
            if (total < best) { best = total; bestOrd = cur; }
            if ((++hojas & ((1 << 20) - 1)) == 0 && ms_desde(t0) > limite_ms) timeout = true;
            return;
        }
        for (int j = 1; j <= k; ++j) {
            if (usado[j]) continue;
            usado[j] = 1;
            cur.push_back(j);
            dfs(prof + 1, j, costo + c[ultimo][j]);
            cur.pop_back();
            usado[j] = 0;
        }
    }
};

static Sol fuerza_bruta(const std::vector<std::vector<double>>& c, int k, double limite_s) {
    Brute b(c, k, limite_s);
    b.dfs(0, 0, 0.0);
    Sol s;
    if (b.timeout) { s.ok = false; return s; }
    s.costo = b.best;
    s.orden.push_back(0);
    for (int v : b.bestOrd) s.orden.push_back(v);
    s.orden.push_back(0);
    return s;
}

// ---------------- Held-Karp ----------------
// dp[S][j] = menor costo de salir del inicio, visitar exactamente los lugares de S y terminar en j (j en S)
// dp[{j}][j] = c[0][j];  dp[S][j] = min_{i en S\{j}} dp[S\{j}][i] + c[i][j]
// Optimo = min_j dp[todos][j] + c[j][0]
static Sol held_karp(const std::vector<std::vector<double>>& c, int k, double limite_s) {
    Sol s;
    if (k > 22) { s.ok = false; return s; }               // 2^22 * 22 doubles ya son ~740 MB
    size_t N = (size_t)1 << k;
    std::vector<double> dp(N * k, INF);
    std::vector<int8_t> padre(N * k, -1);
    auto t0 = Clock::now();
    for (int j = 0; j < k; ++j) dp[((size_t)1 << j) * k + j] = c[0][j + 1];
    for (size_t S = 1; S < N; ++S) {
        for (int j = 0; j < k; ++j) {
            if (!(S >> j & 1)) continue;
            size_t Sp = S & ~((size_t)1 << j);
            if (Sp == 0) continue;
            double mejor = INF;
            int arg = -1;
            for (int i = 0; i < k; ++i) {
                if (!(Sp >> i & 1)) continue;
                double v = dp[Sp * k + i] + c[i + 1][j + 1];
                if (v < mejor) { mejor = v; arg = i; }
            }
            dp[S * k + j] = mejor;
            padre[S * k + j] = (int8_t)arg;
        }
        if ((S & 0xFFFF) == 0 && ms_desde(t0) > limite_s * 1000.0) { s.ok = false; return s; }
    }
    size_t full = N - 1;
    double mejor = INF;
    int ult = -1;
    for (int j = 0; j < k; ++j) {
        double v = dp[full * k + j] + c[j + 1][0];
        if (v < mejor) { mejor = v; ult = j; }
    }
    s.costo = mejor;
    // reconstruccion del recorrido con la tabla de padres
    std::vector<int> rev;
    size_t S = full;
    int j = ult;
    while (j != -1) {
        rev.push_back(j + 1);
        int p = padre[S * k + j];
        S &= ~((size_t)1 << j);
        j = p;
    }
    s.orden.push_back(0);
    for (int i = (int)rev.size() - 1; i >= 0; --i) s.orden.push_back(rev[i]);
    s.orden.push_back(0);
    return s;
}

// ---------------- heuristica voraz: vecino mas cercano ----------------
static Sol vecino_mas_cercano(const std::vector<std::vector<double>>& c, int k) {
    Sol s;
    std::vector<char> usado(k + 1, 0);
    int actual = 0;
    s.orden.push_back(0);
    s.costo = 0;
    for (int paso = 0; paso < k; ++paso) {
        int mejor = -1;
        for (int j = 1; j <= k; ++j)
            if (!usado[j] && (mejor == -1 || c[actual][j] < c[actual][mejor])) mejor = j;
        usado[mejor] = 1;
        s.costo += c[actual][mejor];
        s.orden.push_back(mejor);
        actual = mejor;
    }
    s.costo += c[actual][0];
    s.orden.push_back(0);
    return s;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Uso: tsp <matriz.csv> <instancias.csv> <salida.csv> <brute,hk,greedy> [limite_s]\n";
        return 1;
    }
    double limite_s = argc >= 6 ? std::atof(argv[5]) : 300.0;
    std::vector<std::string> algos = split(argv[4]);

    // matriz
    std::ifstream fm(argv[1]);
    if (!fm) { std::cerr << "No se pudo abrir " << argv[1] << "\n"; return 1; }
    std::vector<std::vector<double>> M;
    std::string line;
    std::getline(fm, line);
    while (std::getline(fm, line)) {
        if (line.empty()) continue;
        auto c = split(line);
        std::vector<double> fila;
        for (size_t i = 1; i < c.size(); ++i) fila.push_back(std::atof(c[i].c_str()));
        M.push_back(fila);
    }

    std::ifstream fi(argv[2]);
    if (!fi) { std::cerr << "No se pudo abrir " << argv[2] << "\n"; return 1; }
    std::ofstream out(argv[3]);
    out.setf(std::ios::fixed);
    out.precision(4);
    out << "inst,k,algoritmo,tiempo_ms,costo_s,estado,orden\n";
    std::getline(fi, line);  // inst,k,inicio,lugares
    while (std::getline(fi, line)) {
        if (line.empty()) continue;
        auto c = split(line);
        std::string inst = c[0];
        int k = std::atoi(c[1].c_str());
        int inicio = std::atoi(c[2].c_str());
        std::vector<int> lugares;
        {
            std::istringstream ss(c[3]);
            int x;
            while (ss >> x) lugares.push_back(x);
        }
        // submatriz local: 0 = inicio, 1..k = lugares
        std::vector<int> idx = {inicio};
        idx.insert(idx.end(), lugares.begin(), lugares.end());
        std::vector<std::vector<double>> sub(k + 1, std::vector<double>(k + 1));
        for (int a = 0; a <= k; ++a)
            for (int b = 0; b <= k; ++b) sub[a][b] = M[idx[a]][idx[b]];

        for (const std::string& alg : algos) {
            Sol sol;
            double ms = 0;
            bool ok;
            if (alg == "brute") ok = medir_mediana([&]() { sol = fuerza_bruta(sub, k, limite_s); return sol.ok; }, ms);
            else if (alg == "hk") ok = medir_mediana([&]() { sol = held_karp(sub, k, limite_s); return sol.ok; }, ms);
            else if (alg == "greedy") ok = medir_mediana([&]() { sol = vecino_mas_cercano(sub, k); return true; }, ms);
            else { std::cerr << "Algoritmo desconocido: " << alg << "\n"; return 1; }
            out << inst << ',' << k << ',' << alg << ',';
            if (!ok) {
                out << ",,no_completada,\n";
            } else {
                out << ms << ',' << sol.costo << ",ok,";
                for (size_t i = 0; i < sol.orden.size(); ++i) out << (i ? " " : "") << idx[sol.orden[i]];
                out << "\n";
            }
            std::cerr << inst << " k=" << k << " " << alg << " " << (ok ? ms : -1) << " ms\n";
        }
    }
    return 0;
}
