#include "random_graph.hpp"
#include <random>
#include <set>
#include <algorithm>

Graph generate_random_graph(int vertices, int edges, int seed)
{
    Graph g(vertices, false); // undirected

    // Initialize RNG
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, vertices - 1);

    std::set<std::pair<int,int>> used;
    int added = 0;

    while (added < edges) 
    {
        int u = dist(rng);
        int v = dist(rng);

        // Skip self-loops
        if (u == v) continue;

        // Store edge with smaller vertex first to avoid duplicates
        auto e = std::make_pair(std::min(u, v), std::max(u, v));
        if (used.count(e)) continue; // skip duplicates

        g.addEdge(u, v);
        used.insert(e);
        ++added;
    }

    return g;
}
