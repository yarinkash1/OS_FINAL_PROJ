/*

@author : Roy Meoded
@author : Yarin Keshet

@date: 14-10-2025

@description: This file contains the declaration of the MSTWeight class, which implements 
the algorithm to find the total weight of the Minimum Spanning Tree (MST) using Kruskal's algorithm.
Using also the DSU (Disjoint Set Union) data structure to efficiently manage and merge sets of vertices.
*/



#pragma once

#include "../part_1/graph_impl.hpp"

#include <vector>
#include <algorithm>

class MSTWeight 
{
public:
    // Returns the total weight of the MST
    int findMSTWeight(const Graph& graph);
};
