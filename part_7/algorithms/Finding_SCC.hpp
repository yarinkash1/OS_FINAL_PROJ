/*
@author : Roy Meoded
@author : Yarin Keshet

@date: 14-10-2025

@description: This file contains the declaration of the FindingSCC class, which implements
the algorithm to find Strongly Connected Components (SCCs) in a directed graph using Kosaraju's algorithm.
*/
#pragma once

#include "../part_1/graph_impl.hpp"
#include <vector>
#include <stack>
#include <algorithm>

class FindingSCC 
{
public:
    // Returns a vector of SCCs, each SCC is a vector of vertex indices
    std::vector<std::vector<int>> findSCCs(const Graph& graph);
};