/*
@author : Roy Meoded
@author : Yarin Keshet

@date: 18-10-2025

@description: This file contains the AlgorithmFactory class that creates instances of different graph algorithms
based on a given identifier. 
It uses the Factory Design Pattern to encapsulate the creation logic.
*/


#pragma once
#include <memory>
#include <string>
#include "IAlgorithm.hpp"
#include "MaxFlowAlgo.hpp"
#include "CliquesAlgo.hpp"
#include "SCCAlgo.hpp"
#include "MSTAlgo.hpp"
#include <algorithm>

class AlgorithmFactory 
{
public:
    static std::unique_ptr<IAlgorithm> create(const std::string& id);
};
