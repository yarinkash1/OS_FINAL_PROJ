#include <iostream>
#include "graph_impl.hpp"

int main() 
{
    // ===== Undirected graph =====
    Graph g(5, false);

    // Add valid edges
    g.addEdge(0, 1);
    g.addEdge(0, 4);

    // Invalid edge (should throw out_of_range)
    try 
    {
        g.addEdge(-1, 2);
    }
    catch (const std::out_of_range& e) 
    {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    // Valid get_neighbors
    const auto& neighbors = g.get_neighbors(0);
    std::cout << "Neighbors of 0: ";
    for (int v : neighbors) std::cout << v << " ";
    std::cout << std::endl;

    // Invalid get_neighbors (should throw out_of_range)
    try {
        g.get_neighbors(10);
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    // Test getters
    std::cout << "Vertices: " << g.get_vertices() << std::endl;
    std::cout << "Edges: " << g.get_edges() << std::endl;
    (void)g.getAdjList();
    (void)g.get_capacity();

    // Test is_edge function
    std::cout << "is_edge(0,1): " << g.is_edge(0,1) << std::endl;
    std::cout << "is_edge(2,4): " << g.is_edge(2,4) << std::endl;

    // ===== Directed graph =====
    Graph d(3, true);

    // Add valid directed edges
    d.addEdge(0, 1, 5);
    d.addEdge(1, 2, 0);

    // Invalid edge with negative capacity (should throw invalid_argument)
    try 
    {
        d.addEdge(2, 2, -7);
    }
    catch (const std::invalid_argument& e) 
    {
        std::cout << "Caught invalid_argument: " << e.what() << std::endl;
    }

    // Test getters for directed graph
    std::cout << "Directed Vertices: " << d.get_vertices() << std::endl;
    std::cout << "Directed Edges: " << d.get_edges() << std::endl;
    (void)d.getAdjList();
    (void)d.get_capacity();

    // Test is_edge for directed graph
    std::cout << "is_edge(1,2): " << d.is_edge(1,2) << std::endl;
    std::cout << "is_edge(2,1): " << d.is_edge(2,1) << std::endl;

    // Call print() to cover printing function
    std::cout << "Directed graph adjacency list:\n";
    d.print();

    return 0;
}
