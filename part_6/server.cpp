#include "server.hpp"
#include <cstdlib> // getenv

// Wrapper for exit that can be suppressed during cumulative coverage runs.
static void coverage_exit(int code)
{
    if (std::getenv("SUPPRESS_EXIT"))
    {
        // Mark that we would have exited; still count this line.
        return;
    }
    exit(code);
}

void run_server()
{
    // Main variables for socket setup & client handling.
    int new_socket; // socket for the client connection
    struct sockaddr_in address;
    int opt = 1; // option for setsockopt
    int addrlen = sizeof(address);
    char buffer[4096] = {0}; // buffer to store incoming data

    // Create socket file descriptor:
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bool force_socket_err     = std::getenv("COV_FORCE_SOCKET_ERROR");
    bool force_setsockopt_err = std::getenv("COV_FORCE_SETSOCKOPT_ERROR");
    bool force_bind_err       = std::getenv("COV_FORCE_BIND_ERROR");
    bool force_listen_err     = std::getenv("COV_FORCE_LISTEN_ERROR");
    bool force_select_err     = std::getenv("COV_FORCE_SELECT_ERROR");
    bool force_accept_err     = std::getenv("COV_FORCE_ACCEPT_ERROR");

    if ((server_fd == 0) || force_socket_err)
    {
        perror("socket failed");
        coverage_exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) || force_setsockopt_err)
    {
        perror("setsockopt");
        coverage_exit(EXIT_FAILURE);
    }

    // Initialize address structure:
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 || force_bind_err)
    {
        perror("bind failed");
        coverage_exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0 || force_listen_err)
    {
        perror("listen");
        coverage_exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    // Main server loop to accept and handle client connections:
    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

    // Timeout: default 30s, override with FAST_TIMEOUT env (used for quick gcov runs)
        struct timeval timeout;
        const char* fast_env = std::getenv("FAST_TIMEOUT");
        long timeout_sec = 30;
        if (fast_env) {
            char* endp = nullptr;
            long v = strtol(fast_env, &endp, 10);
            if (endp != fast_env && v >= 0 && v < 3600) {
                timeout_sec = v;
            }
        }
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity == 0)
        {
            std::cout << "No client connected for 30 seconds. Server shutting down." << std::endl;
            break;
        }

        if (activity < 0 || force_select_err)
        {
            perror("select");
            // In original code select failure would break; keep same semantics.
            break;
        }

        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        if (new_socket < 0 || force_accept_err)
        {
            perror("accept");
            // Original code returned (terminating run). For coverage accumulation we break.
            break;
        }

        int valread = read(new_socket, buffer, 4096);
        std::string input(buffer, valread);

        if (input == "EXIT_CLIENT")
        {
            std::cout << "Client exited." << std::endl;
            close(new_socket);
            continue;
        }

        std::cout << "Client connected." << std::endl;

        std::istringstream iss(input);
        int V = 0, E = 0;
        iss >> V >> E;

        std::ostringstream msg;
        if (V <= 0)
        {
            msg << "Error: Number of vertices must be positive.\n";
        }
        else if (E < 0)
        {
            msg << "Error: Number of edges cannot be negative.\n";
        }
        else
        {
            Graph g(V, false); // false for undirected
            bool valid = true;
            for (int i = 0; i < E; ++i)
            {
                int u, v;
                iss >> u >> v;
                if (u < 0 || v < 0 || u >= V || v >= V)
                {
                    msg << "Error: Invalid edge (" << u << ", " << v << "). Vertices must be in range 0 to " << V - 1 << ".\n";
                    valid = false;
                    break;
                }
                g.addEdge(u, v);
            }
            if (valid)
            {
                std::ostringstream oss;
                std::streambuf *old_cout = std::cout.rdbuf(oss.rdbuf());
                EulerCircle ec(g);
                ec.findEulerianCircuit();
                std::cout.rdbuf(old_cout);

                msg << "Welcome to the Euler Graph Server!\n";
                msg << "Vertices: " << V << "\n";
                msg << "Edges: " << E << "\n";
                msg << oss.str();
            }
        }
        std::string result = msg.str();

        std::cout << "Received from client:\n" << input << std::endl;
        std::cout << "Response sent:\n" << result << std::endl;

        send(new_socket, result.c_str(), result.size(), 0);
        close(new_socket);
    }
    return;
}

int main()
{
    run_server();
    return 0;
}




