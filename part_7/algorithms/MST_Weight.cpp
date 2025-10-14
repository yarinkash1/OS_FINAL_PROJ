#include "MST_Weight.hpp"

/*
This code defines the Edge struct and its comparison operator:
*Edge represents a connection between two vertices (u and v) with a given weight.
*The operator< allows edges to be compared by their weight, so they can be sorted.

Purpose:
Sorting edges by weight is essential for Kruskal's algorithm,
which builds the minimum spanning tree (MST) by repeatedly adding the smallest-weight edge that doesn't form a cycle.
*/
struct Edge 
{
	int u, v, weight;
	bool operator<(const Edge& other) const 
    {
		return weight < other.weight;
	}
};

/*
This code defines the Disjoint Set Union (DSU) class, also known as Union-Find:
*find(x): Finds the representative (root) of the set containing x,
with path compression for efficiency.
*unite(x, y): Merges the sets containing x and y.
Returns true if a merge happened (i.e., they were in different sets),
false if they were already in the same set.
Purpose:
DSU is used in Kruskal's algorithm to efficiently check whether adding an edge would form a cycle.
It keeps track of connected components as edges are added to the MST.
*/
class DSU 
{
	std::vector<int> parent, rank;
public:
	DSU(int n) : parent(n), rank(n, 0) 
    {
		for (int i = 0; i < n; ++i) parent[i] = i;
	}
	int find(int x) 
    {
		if (parent[x] != x) parent[x] = find(parent[x]);
		return parent[x];
	}
	bool unite(int x, int y) 
    {
		int xr = find(x), yr = find(y);
		if (xr == yr) return false;
		if (rank[xr] < rank[yr]) parent[xr] = yr;
		else if (rank[xr] > rank[yr]) parent[yr] = xr;
		else { parent[yr] = xr; rank[xr]++; }
		return true;
	}
};

/*
This function implements Kruskal's algorithm to find the total weight of the Minimum Spanning Tree (MST):
*It collects all edges from the graph, avoiding duplicates.
*Sorts the edges by weight (smallest first).
*Uses the DSU (Union-Find) structure to add edges one by one, only if they connect different components (to avoid cycles).
*Adds the edge's weight to the total MST weight.
*Stops when enough edges have been added to connect all vertices (n - 1 edges for n vertices).
*Returns the total MST weight.
*/
int MSTWeight::findMSTWeight(const Graph& graph) 
{
	int n = graph.get_vertices();
	std::vector<Edge> edges;
	const auto& adj = graph.getAdjList();
	const auto& capacity = graph.get_capacity();
	for (int u = 0; u < n; ++u) 
    {
		for (int v : adj[u]) 
        {
			if (u < v) { // Avoid duplicate edges in undirected graph
				edges.push_back({u, v, capacity[u][v]});
			}
		}
	}
	std::sort(edges.begin(), edges.end());
	DSU dsu(n);
	int mst_weight = 0;
	int edges_used = 0;
	for (const auto& e : edges) 
    {	
		// Try to unite the sets of u and v
		if (dsu.unite(e.u, e.v)) 
        {
			mst_weight += e.weight;
			edges_used++;
			if (edges_used == n - 1) break;
		}
	}
	return mst_weight;
}