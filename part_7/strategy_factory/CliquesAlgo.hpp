/*
@author: Roy Meoded
@author: Yarin Keshet

@date: 18-10-2025

@description: This file contains the CliquesAlgo class that implements the IAlgorithm interface
to find the number of cliques of size k in a given graph.
*/



#pragma once
#include "IAlgorithm.hpp"
#include "Finding_Num_Cliques.hpp"

class CliquesAlgo : public IAlgorithm 
{
public:
    std::string id() const override
    {
        return "CLIQUES";
    }
    std::string run(const Graph& g, const std::unordered_map<std::string,int>& params) override 
    {
        int k = params.count("K") ? params.at("K") : 3; // Reads K from params (defaults to 3)
        FindingNumCliques algo; // Instantiates the algorithm class
        int res = algo.countCliques(g, k); // Executes the algorithm
        return "RESULT " + std::to_string(res); // Returns the result
    }
};
