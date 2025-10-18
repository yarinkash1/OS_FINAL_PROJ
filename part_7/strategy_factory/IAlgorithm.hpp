/*
@ author : Roy Meoded
@ author : Yarin Keshet

@ date: 18-10-2025

@ description: This file contains the interface IAlgorithm for different graph algorithms.
Each algorithm must implement the id() method to return a stable identifier,
and the run() method to execute the algorithm on a given graph with parameters.
*/

#pragma once

#include <string>
#include <unordered_map>

#include "../../part_1/graph_impl.hpp"

// Strategy interface for algorithms:
class IAlgorithm// Abstract base class (interface) for all algorithms
{
public:
    virtual ~IAlgorithm() = default; // Virtual destructor
    virtual std::string id() const = 0; // Declares a pure virtual getter that returns a stable identifier for the algorithm,
    // without modifying the object

    // A pure virtual method all algorithms must implement to execute on a graph:
    virtual std::string run(const Graph& g, const std::unordered_map<std::string, int>& params) = 0; 
};
