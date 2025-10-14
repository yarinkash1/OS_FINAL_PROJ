#include "Finding_Num_Cliques.hpp"


// Helper function to check if a set of vertices forms a clique
static bool isClique(const Graph& graph, const std::vector<int>& vertices) 
{
	for (size_t i = 0; i < vertices.size(); ++i) 
    {
		for (size_t j = i + 1; j < vertices.size(); ++j) 
        {
			if (!graph.is_edge(vertices[i], vertices[j])) 
            {
				return false;
			}
		}
	}
	return true;
}

/*
This function recursively generates all possible combinations of k vertices from the graph:
*If the current combination (current) has k vertices,
it checks if they form a clique and returns 1 if true, 0 otherwise.
*Otherwise, it tries adding each vertex (from start to n-1) to current,
recursing to build larger combinations.
*After each recursive call, it removes the last vertex to backtrack.
*The sum of all recursive calls gives the total number of k-cliques.
*/

static int countCliquesRecursive(const Graph& graph, int k, int start, std::vector<int>& current) 
{
	if (static_cast<int>(current.size()) == k) 
    {
		return isClique(graph, current) ? 1 : 0;
	}
	int count = 0;
	int n = graph.get_vertices();
	for (int v = start; v < n; ++v) 
    {
		current.push_back(v);
		count += countCliquesRecursive(graph, k, v + 1, current); //Calling itself recursively to build combinations.
		current.pop_back();
	}
	return count;
}

// Count cliques of size k
int FindingNumCliques::countCliques(const Graph& graph, int k) 
{
	std::vector<int> current; // creates an empty vector current to hold the current combination of vertices.
	return countCliquesRecursive(graph, k, 0, current); // Recursively builds all possible groups of k vertices and counts those that form cliques.
    // The result is the total number of k-cliques in the graph.
}

