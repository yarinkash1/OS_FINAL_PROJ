/*
@author : Roy Meoded
@author : Yarin Keshet

@date: 20-10-2025

@description: This file contains the client-side implementation of the application, including
functions for handling user input, connecting to the server, sending requests, and receiving responses.
Defined functions:
- println_rule(): Prints a separator line to the console.
- prompt_int(msg, minVal, maxVal): Prompts the user for an integer input within a specified range.
- run_client(argc, argv): Main client function to connect to the server and handle user interactions.
- getline_with_timeout(out, timeout_ms): Reads a line from stdin with a timeout.
*/

#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <cstdlib>
#include <cctype>

#ifndef PORT
#define PORT 9090
#endif

// Reuse Part 8's random graph interface; implementation linked via makefile.
#include "../../part_8/include/random_graph.hpp"

void println_rule();
int prompt_int(const std::string& msg, int minVal, int maxVal);
int run_client(int argc, char* argv[]);