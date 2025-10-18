/*
@author: Roy Meoded
@author: Yarin Keshet

@date: 18-10-2025

@description: This file contains the SCCAlgo class that implements the IAlgorithm interface
to find the strongly connected components (SCC) in a given graph.

*/

#pragma once
#include "IAlgorithm.hpp"
#include "Finding_SCC.hpp"

class SCCAlgo : public IAlgorithm 
{
public:
    std::string id() const override 
    {
        return "SCC"; 
    }
    std::string run(const Graph& g, const std::unordered_map<std::string,int>&) override 
    {
        FindingSCC algo; // Instantiates the algorithm class
        auto sccs = algo.findSCCs(g); // Executes the algorithm
        return "RESULT " + std::to_string((int)sccs.size()); // Returns the result
    }
};
