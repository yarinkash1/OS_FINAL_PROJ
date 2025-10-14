/*
@author : Roy Meoded
@author : Yarin Keshet

@date: 14-10-2025

@description: This file contains the declaration of the FindingNumCliques class, which provides 
a method to count the number of cliques of a given size in a graph.
*/

#pragma once

#include "../part_1/graph_impl.hpp"

#include <vector>
#include <algorithm>

class FindingNumCliques 
{
public:
    // Counts the number of cliques of size k in the given graph
    int countCliques(const Graph& graph, int k);
};