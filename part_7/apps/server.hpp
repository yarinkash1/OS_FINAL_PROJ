/*
@author: Roy Meoded
@author: Yarin Keshet

@date: 18-10-2025

@description: This file contains the server-side implementation of the application, including
functions for handling client requests, processing data, and sending responses.
Defined functions:
- recv_all_lines(fd, out): Receives all lines from a client socket.
- send_response(fd, body, ok): Sends a response back to the client.
- run_server(argc, argv): Main server function to start the server and accept client connections.

*/



#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <csignal>
#include "Finding_Max_Flow.hpp"
#include "../part_1/graph_impl.hpp"
#include "../strategy_factory/AlgorithmFactory.hpp"

#define PORT 9090 // Default port

bool recv_all_lines(int fd, std::string &out);
void send_response(int fd, const std::string &body, bool ok = true);
int run_server(int argc, char* argv[]);
