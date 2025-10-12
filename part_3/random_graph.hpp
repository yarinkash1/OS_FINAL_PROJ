/*
@author Roy Meoded
@author Yarin Keshet
@date 12-10-2025

@description: Header file for random graph generation, declares function to generate a random undirected graph
with specified number of vertices, edges, and a seed for randomness.
This file includes the Graph class from part_1/graph_impl.hpp to utilize its graph representation and methods.
*/


#pragma once
#include "../part_1/graph_impl.hpp"

// Generate a random undirected graph with given number of vertices, edges, and seed
Graph generate_random_graph(int vertices, int edges, int seed);
