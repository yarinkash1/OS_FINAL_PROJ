#include "../part_1/graph_impl.hpp"
#include "../part_2/euler_circle.hpp"
#include "../part_3/random_graph.hpp"

#include <iostream>
#include <unistd.h> // for getopt
#include <cstdlib> // for std::atoi

/*
instructions to run the main demo:
to run the main write this line:
./main -v <vertices> -e <edges> -s $(date +%s)
*/

int main(int argc, char* argv[]) 
{
    int vertices = 0;
    int edges = 0;
    int seed = 0;
    int opt;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "v:e:s:")) != -1) //getopt returns the character of the option found
    {
        switch (opt) 
        {
            case 'v':
            //atoi - ASCII to integer
                vertices = std::atoi(optarg);
                break;
            case 'e':
                edges = std::atoi(optarg);
                break;
            case 's':
                seed = std::atoi(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -v <vertices> -e <edges> -s <seed>\n";
                return 1;
        }
    }

    std::cout << "Vertices: " << vertices << "\n";
    std::cout << "Edges: " << edges << "\n";
    std::cout << "Seed: " << seed << "\n";

    // Next steps: generate random graph and run EulerCircle...
    Graph g = generate_random_graph(vertices, edges, seed);
    EulerCircle ec(g);
    ec.findEulerianCircuit();

    return 0;
}

