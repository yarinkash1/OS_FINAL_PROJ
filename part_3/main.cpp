#include "../part_1/graph_impl.hpp"
#include "../part_2/euler_circle.hpp"
#include "random_graph.hpp"

#include <iostream>
#include <unistd.h>   // getopt
#include <cstdlib>    // std::atoi
#include <ctime>      // std::time

static void usage(const char* prog) 
{
    std::cerr << "Usage: " << prog << " -v <vertices> -e <edges> -s <seed>\n";
}

int main(int argc, char* argv[])
{
    // Default values
    int vertices = 0;
    int edges    = 0;
    int seed     = 0;
    bool hasV = false, hasE = false, hasS = false;

    int opt;
    while ((opt = getopt(argc, argv, "v:e:s:")) != -1) 
    {
        switch (opt) 
        {
            case 'v': vertices = std::atoi(optarg); hasV = true; break;
            case 'e': edges    = std::atoi(optarg); hasE = true; break;
            case 's': seed     = std::atoi(optarg); hasS = true; break;
            case '?':
            default:
                usage(argv[0]);
                return 1;
        }
    }

    // Require all three arguments, otherwise print usage
    if (!hasV || !hasE || !hasS) 
    {
        usage(argv[0]);
        return 1;
    }

    if (vertices <= 0) 
    {
        std::cerr << "Error: vertices must be positive.\n";
        return 1;
    }
    if (edges < 0) 
    {
        std::cerr << "Error: edges must be non-negative.\n";
        return 1;
    }

    // For a simple undirected graph, maximum possible edges = V*(V-1)/2
    const long long maxEdges = 1LL * vertices * (vertices - 1) / 2;
    if (edges > maxEdges) 
    {
        std::cerr << "Warning: edges > max possible for simple undirected graph ("<< maxEdges << "). Clamping to max.\n";
        edges = (int)maxEdges;
    }

    std::cout << "Vertices: " << vertices << "\n";
    std::cout << "Edges: "    << edges    << "\n";
    std::cout << "Seed: "     << seed     << "\n";

    // Generate random graph and run EulerCircle
    Graph g = generate_random_graph(vertices, edges, seed);

    std::cout << "Graph adjacency list:\n";
    g.print();

    std::cout << "\nChecking for Eulerian circuit:\n";
    EulerCircle ec(g);
    
    ec.findEulerianCircuit();
    

    return 0;
}
