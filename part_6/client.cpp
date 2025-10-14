#include "client.hpp"

void run_client() 
{    
    //serv_addr is a structure used to specify the server’s address for the socket connection
    struct sockaddr_in serv_addr;
    //buffer is a character array used to store data received from the server
    char buffer[4096] = {0};

    
    /*
    The main while loop for user input
    Inside this loop, the client will repeatedly prompt the user for graph parameters and edge definitions.
    */
    while (true) 
    {
        /*
        Socket Creation and Connection:
        This creates a new socket for network communication.
        AF_INET specifies IPv4 addressing.
        SOCK_STREAM means it’s a TCP socket (reliable, connection-oriented).
        The last argument (0) lets the system choose the correct protocol (TCP for SOCK_STREAM).
        */
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) 
        {
            std::cerr << "Socket creation error\n";
            return;
        }

        serv_addr.sin_family = AF_INET; // Sets the address family to IPv4
        serv_addr.sin_port = htons(PORT); // Sets the port number for the server connection(by default 8080)

        /*
        inet_pton: Internet address, presentation to network
        It converts an IP address from its human-readable string form ("127.0.0.1", which is localhost) into its binary form,
        which is needed for network functions.
        &serv_addr.sin_addr is where the binary result will be stored in the serv_addr structure.
        If the conversion fails (returns 0 or negative), it means the address is invalid or not supported
        */
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
        {
            std::cerr << "Invalid address/ Address not supported\n";
            return;
        }

        /*
        connect() is a system call that tries to establish a connection from the client socket (sock) to the server specified by serv_addr.
        (struct sockaddr *)&serv_addr casts the server address structure to the generic sockaddr type required by the function.
        sizeof(serv_addr) gives the size of the address structure.
        If connect() returns a negative value (< 0), the connection attempt failed (e.g., server not running, wrong address, network issues).
        */
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        {
            std::cerr << "Connection Failed\n";
            return;
        }
        
        int V; // V is the number of vertices in the graph
        /*
        Prompt the user for the number of vertices in the graph.
        Check if the input is a valid positive integer.
        Also, ensure that the number of vertices is at least 2.
        */
        while (true) 
        {
            std::cout << "Enter number of vertices (positive integer): ";
            std::string v_input; // Declares a string variable v_input to store the user's input.
            std::getline(std::cin, v_input); // Reads a line of input from the user(the terminal) into the v_input variable.
            /*
            Removes any leading and trailing whitespace characters (spaces, tabs, newlines, carriage returns) from the input.
            This ensures that if the user accidentally adds spaces before or after the number, they are ignored.
            */
            v_input.erase(0, v_input.find_first_not_of(" \t\n\r"));
            v_input.erase(v_input.find_last_not_of(" \t\n\r") + 1);
            /*
            Checks if the input is not empty and that every character in v_input is a digit (0-9).
            std::all_of(v_input.begin(), v_input.end(), ::isdigit) returns true only if all characters are digits.
            The result is stored in valid_int, which will be true if the input is a valid positive integer, and false otherwise.
            */
            bool valid_int = !v_input.empty() && std::all_of(v_input.begin(), v_input.end(), ::isdigit);
            if (!valid_int) 
            {
                std::cout << "Error: Please enter a single positive integer.\n";
                continue;
            }
            V = std::stoi(v_input); // stoi = string to int, converts the string v_input (from the user) into an integer
            if (V <= 1) 
            {
                std::cout << "Error: Number of vertices must be at least 2.\n";
            } 
            else 
            {
                break;
            }
        }

        int max_edges = V * (V - 1) / 2; // Maximum number of edges in a simple graph
        int E; // E is the number of edges in the graph
        /*
        Prompt the user for the number of edges in the graph.
        Check if the input is a valid non-negative integer.
        Also, ensure that the number of edges does not exceed the maximum allowed.
        */
        while (true) 
        {
            std::cout << "Enter number of edges (non-negative integer, max " << max_edges << "): ";
            std::string e_input;
            std::getline(std::cin, e_input);
            e_input.erase(0, e_input.find_first_not_of(" \t\n\r"));
            e_input.erase(e_input.find_last_not_of(" \t\n\r") + 1);
            bool valid_int = !e_input.empty() && std::all_of(e_input.begin(), e_input.end(), ::isdigit);
            if (!valid_int) 
            {
                std::cout << "Error: Please enter a single non-negative integer.\n";
                continue;
            }
            E = std::stoi(e_input);
            if (E < 0 || E > max_edges) 
            {
                std::cout << "Error: Number of edges must be between 0 and " << max_edges << ".\n";
            } 
            else 
            {
                break;
            }
        }

        /*
        Get the edge definitions from the user.
        Each edge is defined by a pair of vertices (u, v).
        *Validates each edge:
            * Checks for correct format and integer values.
            * Prevents self-loops and out-of-range vertices.
            * Prevents duplicate edges (regardless of order).
        * Adds each valid edge to the graph data string.
        */
        std::string graph_data = std::to_string(V) + " " + std::to_string(E) + "\n";
        std::cout << "Enter each edge as: u v (or type 'exit' to quit)\n";
        std::set<std::pair<int, int>> edge_set;
        for (int i = 0; i < E; ++i) 
        {
            int u, v;
            while (true) 
            {
                std::cout << "Edge " << i+1 << ": ";
                std::string edge_line;
                std::getline(std::cin, edge_line);
                edge_line.erase(0, edge_line.find_first_not_of(" \t\n\r"));
                edge_line.erase(edge_line.find_last_not_of(" \t\n\r") + 1);
                if (edge_line == "exit") return;
                size_t space_pos = edge_line.find(' ');
                if (space_pos == std::string::npos) 
                {
                    std::cout << "Error: Please enter two integers separated by a space.\n";
                    continue;
                }
                std::string u_input = edge_line.substr(0, space_pos);
                std::string v_input = edge_line.substr(space_pos + 1);
                v_input.erase(0, v_input.find_first_not_of(" \t\n\r"));
                v_input.erase(v_input.find_last_not_of(" \t\n\r") + 1);
                if (u_input.empty() || v_input.empty() || !std::all_of(u_input.begin(), u_input.end(), ::isdigit) || !std::all_of(v_input.begin(), v_input.end(), ::isdigit)) 
                {
                    std::cout << "Error: Vertices must be integers with no extra characters.\n";
                    continue;
                }
                try 
                {
                    u = std::stoi(u_input);
                    v = std::stoi(v_input);
                } catch (...)// The three dots (...) mean "catch any exception," regardless of its type.
                {
                    std::cout << "Error: Vertices must be integers.\n";
                    continue;
                }
                if (u == v) 
                {
                    std::cout << "Error: Self-loops are not allowed (u must not equal v).\n";
                } 
                else if (u < 0 || v < 0 || u >= V || v >= V) 
                {
                    std::cout << "Error: Vertices must be in range 0 to " << V-1 << ".\n";
                } 
                else 
                {
                    int a = std::min(u, v);
                    int b = std::max(u, v);
                    if (edge_set.count({a, b})) 
                    {
                        std::cout << "Error: Edge already exists. Please enter a new edge.\n";
                        continue;
                    }
                    edge_set.insert({a, b});
                    break;
                }
            }
            graph_data += std::to_string(u) + " " + std::to_string(v) + "\n";
        }

        

        // Sending Graph Data and Receiving Response
        send(sock, graph_data.c_str(), graph_data.size(), 0);
        /*
        Reads up to 4096 bytes of data from the server through the socket sock.
        The data is stored in the buffer array.
        valread will contain the number of bytes actually read (could be less than 4096).
        */
        int valread = read(sock, buffer, 4096);
        /*
        Converts the received bytes in buffer to a string (using only the bytes actually read).
        Prints the server’s response to the user.
        */
        std::cout << "Server response:\n" << std::string(buffer, valread) << std::endl;

        close(sock); // Close the socket

        // Prompting for Another Graph:
        std::string again; // User input for another graph
        while (true) 
        {
            std::cout << "Do you want to enter another graph? (y/n): ";
            std::cin >> again; // Y/y or N/n response from user
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer
            if (again == "y" || again == "Y") 
            {
                break;
            }
            else if (again == "n" || again == "N") 
            {
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock >= 0) {
                    serv_addr.sin_family = AF_INET;
                    serv_addr.sin_port = htons(PORT);
                    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) > 0) 
                    {
                        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) 
                        {
                            std::string exit_msg = "EXIT_CLIENT";
                            send(sock, exit_msg.c_str(), exit_msg.size(), 0);
                            close(sock);
                        }
                    }
                }
                std::cout << "Goodbye!\n";
                return;
            } 
            else 
            {
                std::cout << "Invalid input. Please enter 'y' or 'n'.\n";
            }
        }
    }
}

int main()
{
    run_client();
    return 0;
}