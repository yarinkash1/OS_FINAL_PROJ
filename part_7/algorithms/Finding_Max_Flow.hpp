/*
@author: Roy Meoded
@author: Yarin Keshset

@date: 14-10-2025

@description: Finding Max Flow in a flow network using Edmonds-Karp algorithm 
(an implementation of Ford-Fulkerson method using BFS)
The algorithm repeatedly finds the shortest augmenting path from source to sink using BFS
and augments the flow along that path until no more augmenting paths can be found.
*/

#pragma once

#include "../part_1/graph_impl.hpp"
#include <queue>
#include <vector>
#include <climits>

class FindingMaxFlow 
{
public:
    int findMaxFlow(Graph& g, int source, int sink);
};