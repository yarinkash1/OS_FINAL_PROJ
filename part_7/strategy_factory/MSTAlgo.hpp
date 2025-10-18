/*
@author: Roy Meoded
@author: Yarin Keshet

@date: 18-10-2025

@description: This file contains the MSTAlgo class that implements the IAlgorithm interface
to find the weight of the Minimum Spanning Tree (MST) in a given graph.
*/

#pragma once
#include "IAlgorithm.hpp"
#include "MST_Weight.hpp"

class MSTAlgo : public IAlgorithm 
{
public:
    std::string id() const override 
    { 
        return "MST"; 
    }
    std::string run(const Graph& g, const std::unordered_map<std::string,int>&) override 
    {
        MSTWeight algo; // Instantiates the algorithm class
        int res = algo.findMSTWeight(g); // Executes the algorithm
        return "RESULT " + std::to_string(res); // Returns the result
    }
};
