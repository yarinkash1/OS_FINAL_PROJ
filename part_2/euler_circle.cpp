#include "euler_circle.hpp"

void EulerCircle::findEulerianCircuit() 
{
    std::vector<std::vector<int>> adjList = g.getAdjList(); // copy the adjacency list
    int V = adjList.size();
    bool isEulerian = true;
    // Check for undirected graph: all degrees must be even
    for (int u = 0; u < V; ++u) 
    {
        if (adjList[u].size() % 2 != 0) 
        {
            isEulerian = false;
            std::cout << "Vertex " << u << " has odd degree: " << adjList[u].size() << std::endl;
        }
    }
    if (!isEulerian) 
    {
        std::cout << "No Eulerian circuit exists: not all vertices have even degree." << std::endl;
        return;
    }

    // If all degrees are even, run Hierholzer's algorithm
    std::vector<int> circuit; // initialize
    hierholzer(adjList, 0, circuit);
    // Print the Eulerian circuit
    std::cout << "Eulerian circuit: ";
    for (int v : circuit) 
    {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

/*
Hierholzer’s algorithm finds an Eulerian circuit by:

1. Starting at any vertex.
2. Following edges (removing them as you go) until you return to the starting vertex, forming a cycle.
3. If there are unused edges, start a new cycle from a vertex in the existing cycle that has unused edges,
and merge the cycles.
4. Repeat until all edges are used.

Result: a path that visits every edge exactly once and returns to the start.

*/
void EulerCircle::hierholzer(std::vector<std::vector<int>>& adjList, int start, std::vector<int>& circuit) 
{
    std::vector<int> stack;
    stack.push_back(start);
    while (!stack.empty()) 
    {
        int u = stack.back();
        if (!adjList[u].empty()) 
        {
            int v = adjList[u].back(); // saves the v vertex
            adjList[u].pop_back(); // removes the last neighbor (vertex) from the adjacency list of vertex u(the edge uv is removed)
            // Remove the edge in the other direction for undirected graphs
            for (auto it = adjList[v].begin(); it != adjList[v].end(); ++it) 
            {
                if (*it == u) 
                {
                    adjList[v].erase(it);
                    break;
                }
            }
            stack.push_back(v);
        }
        else 
        {
            circuit.push_back(u);
            stack.pop_back();
        }
    }
    std::reverse(circuit.begin(), circuit.end()); // reverse the circuit to get the correct order
}
