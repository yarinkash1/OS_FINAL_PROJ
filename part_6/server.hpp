/*
@author Roy Meoded
@author Yarin Keshset

@date 13-10-2025

@description: Header file for server implementation--> with TCP server, and PORT 8080 as default.
The server accecpt connection, reading request, build graph, and running Euler , and return answer.
Using the important method run_server():
- run_server(): Main server loop to accept and handle client connections.
After 30 seconds of no client connection, the server will shut down.
*/

#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include "../part_1/graph_impl.hpp"
#include "../part_2/euler_circle.hpp"

#define PORT 8080 // Default port

void run_server(); 
