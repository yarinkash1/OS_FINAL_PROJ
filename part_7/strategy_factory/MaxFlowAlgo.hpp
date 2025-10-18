/*
@author: Roy Meoded
@author: Yarin Keshet

@date: 18-10-2025

@description: This file contains the MaxFlowAlgo class that implements the IAlgorithm interface
to find the maximum flow in a given graph using the Edmonds-Karp algorithm.

*/

#pragma once
#include "IAlgorithm.hpp"
#include "Finding_Max_Flow.hpp"

class MaxFlowAlgo : public IAlgorithm 
{
public:
    std::string id() const override
    {
        return "MAX_FLOW"; 
    }
    std::string run(const Graph& g, const std::unordered_map<std::string,int>& params) override 
    {
        int src = params.count("SRC") ? params.at("SRC") : 0; // Reads SRC from params (defaults to 0)
        int sink = params.count("SINK") ? params.at("SINK") : g.get_vertices()-1; // Reads SINK from params (defaults to last vertex)
        FindingMaxFlow algo; // Instantiates the algorithm class
        Graph gCopy = g; // make a modifiable copy for algorithms that mutate the graph
        int res = algo.findMaxFlow(gCopy, src, sink); // Executes the algorithm
        return "RESULT " + std::to_string(res); // Returns the result
    }
};
