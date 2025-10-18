/*
@author : Roy Meoded
@author : Yarin Keshet

@date: 18-10-2025

@description: This file contains helper UI/IO utilities for the client application, including functions for
 user input, output formatting, and network communication.
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
#include <vector>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <poll.h>
#include <cstdlib>

#define PORT 9090

// Helper UI/IO utilities for the client
void println_rule();
int prompt_int(const std::string& msg, int minVal, int maxVal);
int run_client(int argc, char* argv[]);
bool getline_with_timeout(std::string &out, int timeout_ms);
