// common.hpp - utilidades compartidas por los tres programas en C++
#pragma once
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using ll = long long;

// ---------- lectura de CSV simple (sin comillas ni comas dentro de los campos) ----------
inline std::vector<std::string> split(const std::string& s, char sep = ',') {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == sep) { out.push_back(cur); cur.clear(); }
        else if (c != '\r') cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

// ---------- cronometro ----------
using Clock = std::chrono::steady_clock;
inline double ms_desde(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// Mediana de un vector (se modifica el orden)
inline double mediana(std::vector<double>& v) {
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n == 0) return 0.0;
    return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

// Protocolo experimental: 5 repeticiones y mediana; si la primera ejecucion supera
// 60 s se mide una sola vez. "f" devuelve false si la ejecucion paso el limite de tiempo.
// Devuelve false si alguna ejecucion no se completo.
template <class F>
bool medir_mediana(F f, double& mediana_ms, int reps = 5) {
    std::vector<double> t;
    for (int r = 0; r < reps; ++r) {
        auto t0 = Clock::now();
        bool ok = f();
        double ms = ms_desde(t0);
        if (!ok) return false;
        t.push_back(ms);
        if (r == 0 && ms > 60000.0) break;  // ejecuciones de mas de un minuto: una sola vez
    }
    mediana_ms = mediana(t);
    return true;
}

// ---------- grafo en listas de adyacencia compactas (CSR) ----------
struct Graph {
    int V = 0;
    std::vector<int> off;    // off[u] .. off[u+1]-1 son las aristas que salen de u
    std::vector<int> to;     // destino de cada arista
    std::vector<double> w;   // peso (tiempo de viaje en segundos)
};

// Carga data/processed/nodes.csv (solo cuenta nodos) y edges.csv (u,v,length_m,time_s,...)
inline Graph cargar_grafo(const std::string& dir) {
    Graph g;
    {
        std::ifstream f(dir + "/nodes.csv");
        if (!f) { std::cerr << "No se pudo abrir " << dir << "/nodes.csv\n"; std::exit(1); }
        std::string line;
        std::getline(f, line);
        while (std::getline(f, line)) if (!line.empty()) g.V++;
    }
    std::vector<int> U, W;
    std::vector<double> T;
    {
        std::ifstream f(dir + "/edges.csv");
        if (!f) { std::cerr << "No se pudo abrir " << dir << "/edges.csv\n"; std::exit(1); }
        std::string line;
        std::getline(f, line);
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            auto c = split(line);
            U.push_back(std::atoi(c[0].c_str()));
            W.push_back(std::atoi(c[1].c_str()));
            T.push_back(std::atof(c[3].c_str()));
        }
    }
    size_t E = U.size();
    g.off.assign(g.V + 1, 0);
    for (size_t i = 0; i < E; ++i) g.off[U[i] + 1]++;
    for (int i = 0; i < g.V; ++i) g.off[i + 1] += g.off[i];
    g.to.resize(E);
    g.w.resize(E);
    std::vector<int> pos(g.off.begin(), g.off.end() - 1);
    for (size_t i = 0; i < E; ++i) {
        int p = pos[U[i]]++;
        g.to[p] = W[i];
        g.w[p] = T[i];
    }
    return g;
}
