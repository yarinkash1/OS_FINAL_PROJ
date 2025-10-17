#include "Finding_Max_Flow.hpp"

int FindingMaxFlow::findMaxFlow(Graph& g, int source, int sink) 
{
    int V = g.get_vertices();

    // Copy the capacity matrix from the graph
    // Capacity[u][v] = capacity of edge u->v
    std::vector<std::vector<int>> residual = g.get_capacity();

    int maxFlow = 0;
    std::vector<int> parent(V); // To store the path

    /*               
    Breadth-First Search (BFS) to find an augmenting path using a lambda function:

    *auto lets the compiler deduce the type.
    *[&] means the lambda captures all local variables by reference (so it can use and modify them).
    *(int s, int t) are the parameters: s is the source node, t is the sink node.
    *-> bool means the lambda returns a boolean value.

    What does the lambda do?

    *It performs a Breadth-First Search (BFS) from node s to node t in the residual graph.
    *It tries to find an augmenting path (a path with available capacity).
    *If it finds such a path, it returns true; otherwise, it returns false.
    *It also fills the parent vector so it's possible to reconstruct the path later.
    */
    auto bfs = [&](int s, int t) -> bool 
    {
        /*
        This sets every element of the parent vector to -1.
        parent is used to keep track of the path found by BFS.
        Setting all values to -1 means "no parent assigned yet" (i.e., the node hasn't been visited).
        */
        std::fill(parent.begin(), parent.end(), -1); 
        std::queue<int> q; // BFS queue
        q.push(s); // Start BFS from source
        parent[s] = s; // Mark source as visited
        while (!q.empty()) 
        {
            /*
            q.front() gets the first element in the queue q (the current node to process in BFS).
            int u = q.front(); assigns that node to the variable u.
            q.pop(); removes that node from the queue, so it's possible to process the next one in the following iteration.
            */
            int u = q.front(); q.pop();
            for (int v = 0; v < V; ++v) 
            {
                // Check if the parent of v is not assigned and there's available capacity
                if (parent[v] == -1 && residual[u][v] > 0) 
                {
                    parent[v] = u; // Set parent of v to u (u-->v in the path)
                    if (v == t) return true; // If we reached the sink, return true
                    q.push(v); // Add v to the BFS queue
                }
            }
        }
        return false; // No augmenting path found (No path from s to t)
    };

    /*
    Repeatedly finds augmenting paths from the source to the sink using BFS.
    Each time a path is found, the loop executes to push more flow.
    The process continues until no more augmenting paths can be found.
    */
    while (bfs(source, sink)) 
    {
        // Find minimum residual capacity along the path:
        int path_flow = INT_MAX; // Initialize path flow to a large value
        for (int v = sink; v != source; v = parent[v]) 
        {
            int u = parent[v];
            path_flow = std::min(path_flow, residual[u][v]);
        }
        // Update residual capacities
        for (int v = sink; v != source; v = parent[v]) 
        {
            int u = parent[v];
            residual[u][v] -= path_flow;
            residual[v][u] += path_flow;
        }
        maxFlow += path_flow;
    }
    return maxFlow;
}