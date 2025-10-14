/*
@author: Roy Meoded
@author: Yarin Keshset

@date: 13-10-2025

@description: Header file for client implementation--> with TCP client, and PORT 8080 as default.
The client connects to the server, sends graph parameters and edges, and receives the Eulerian circuit result.
Using the important method run_client():
- run_client(): Main client loop to prompt user for graph input, send to server, and display server response.
If the server is not reachable, the client will display an error message and exit.
*/

#pragma once

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <limits>
#include <algorithm>
#include <set>

#define PORT 8080 // Default port

void run_client();
