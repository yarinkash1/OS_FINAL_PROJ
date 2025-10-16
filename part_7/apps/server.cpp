#include "server.hpp"

//For SIGINT/SIGTERM handling:
static int g_server_fd = -1;

// Helper function for receiving all lines from a socket
bool recv_all_lines(int fd, std::string &out) 
{
    char buf[1024]; // buffer for receiving data(size is 1024 bytes)
    out.clear(); // clear the output string
    while (true) 
    {
        ssize_t n = recv(fd, buf, sizeof(buf), 0); // receive data from socket
        if (n > 0) // if data is received
        {
            out.append(buf, buf + n); // append received data to output string
            // stop when END line appears:
            if (out.find("\nEND\n") != std::string::npos || out.rfind("\nEND") == out.size() - 4) 
            {
                return true;
            }
            // stop when EXIT line appears:
            if (out.find("\nEXIT\n") != std::string::npos || out == "EXIT\n") 
            {
                return true;
            }
        }
        
        else if (n == 0) 
        {
            // connection closed:
            return !out.empty();
        }
        // connection error:
        else 
        {
            if (errno == EINTR) continue;
            return false;
        }
    }
}

// Helper function for sending a response
void send_response(int fd, const std::string &body, bool ok) 
{
    std::ostringstream oss;
    oss << (ok ? "OK\n" : "ERR\n") << body << "\nEND\n";
    auto s = oss.str();
    (void)send(fd, s.c_str(), s.size(), 0);
}

int run_server(int argc, char *argv[]) 
{
    int port = PORT; // default port is 9090

    // If a port number is provided as a command-line argument, use it:
    if (argc >= 2) 
    {
        port = std::atoi(argv[1]);
        if (port <= 0) 
        {
            port = PORT;
        }
    }

    // Create socket:
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    g_server_fd = srv;

    if (srv < 0) 
    {
        std::perror("socket");
        return 1;
    }

    int opt = 1; // opt = 1 enables the option (allow reuse of a recently used local address/port,
    // mainly to avoid “Address already in use” after restarts).
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //Define the server address struct and bind to the specified port:
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(srv, (sockaddr *)&addr, sizeof(addr)) < 0) 
    {
        std::perror("bind");
        close(srv);
        return 1;
    }

    //Define queue of up to 8 pending connections:
    if (listen(srv, 8) < 0) 
    {
        std::perror("listen");
        close(srv);
        return 1;
    }

    std::cout << "Server listening on port " << port << "...\n";

    while (true) 
    {
        /*
        * Declares a client address struct (cli) and its length (clilen).
        * Calls accept to block until a client connects;
        on success returns a new socket fd for that client.
        * If accept fails due to an interrupt (EINTR), retry the loop;
        otherwise log the error and break out of the accept loop.
        */
        sockaddr_in cli{};
        socklen_t clilen = sizeof(cli);
        int fd = accept(srv, (sockaddr *)&cli, &clilen);
        if (fd < 0) 
        {
            if (errno == EINTR) continue;
            std::perror("accept");
            break;
        }

    // Log client connection details:
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Client connected from " << ipstr << ":" << ntohs(cli.sin_port) << "\n";

        while (true) 
        {
            std::string req; // Allocates a buffer string req to hold the incoming request.
            /*
            Read from the client until a full request is received (it stops on END or EXIT) or the socket closes/errors.
            */
            if (!recv_all_lines(fd, req)) 
            {
                close(fd);
                std::cout << "Client disconnected.\n";
                break;
            }
        
        // Check for special commands EXIT and SHUTDOWN:
          auto has_cmd = [](const std::string& s, const char* cmd) {
            std::string c(cmd);
            if (s.rfind(c + "\n", 0) == 0) return true;                
            if (s == c + "\n")        return true;
            if (s.find("\n" + c + "\n") != std::string::npos) return true;
            return false;
        };

        //For EXIT command, close client connection and wait for a new client:
        if (has_cmd(req, "EXIT")) {
            send_response(fd, "BYE", true);
            close(fd);
            std::cout << "Client disconnected (EXIT).\n";
            break;
        }

        //For SHUTDOWN command, close client connection and shutdown the server:
        if (has_cmd(req, "SHUTDOWN")) {
            send_response(fd, "Server shutting down", true);
            close(fd);
            close(srv);
            std::cout << "Server shutting down on SHUTDOWN request.\n";
            return 0; 
        }



            // Parse request
            std::istringstream iss(req);
            std::string line;
            std::string alg;
            int V = -1;
            int directed = 1; // default directed for MAX_FLOW
            int E = 0;
            struct EdgeLine 
            {
                 int u, v, w; 
            };
            std::vector<EdgeLine> edges;
            int src = -1, sink = -1; int k = -1;

            bool parse_error = false;
            std::string parse_error_msg;
            while (std::getline(iss, line)) 
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                } 
                if (line == "END")
                {
                    break;
                } 
                if (line.rfind("ALG ", 0) == 0) 
                {
                    alg = line.substr(4);
                    // also support: ALG <ID> DIRECTED <0/1>
                    std::istringstream ls(line);
                    std::string tok,idtok;
                    ls >> tok >> idtok;
                    if (!idtok.empty() && idtok != alg)
                    {
                        alg = idtok;
                    } 
                    std::string dirTok; int dirVal;
                    if (ls >> dirTok >> dirVal)
                    {
                        if (dirTok == "DIRECTED") directed = dirVal; 
                    }
                }
                else if (line.rfind("V ", 0) == 0) 
                {
                    // format: V <n> DIRECTED <0/1>
                    std::istringstream ls(line);
                    std::string tok;
                    ls >> tok >> V;
                    if (ls >> tok) 
                    {
                        if (tok == "DIRECTED") 
                        {
                            ls >> directed; 
                        }
                    }
                }
                else if (line.find(' ') != std::string::npos && alg.empty()) 
                {
                    // Try simple header: ALG <ID> DIRECTED <0/1> | or V E as first lines
                    std::istringstream ls(line);
                    std::string a; ls >> a;
                    if (a == "V")
                    {
                        ls >> V >> E; 
                    }
                    else if (a == "ALG") 
                    {
                        ls >> alg; std::string dTok;
                        if (ls >> dTok >> directed) 
                        {/*ok*/} 
                    }
                }
                else if (line.rfind("E ", 0) == 0) 
                {
                    std::istringstream ls(line);
                    char ch;
                    ls >> ch >> E;
                    (void)ch;
                }
                else if (line.rfind("EDGE ", 0) == 0) 
                {
                    std::istringstream ls(line);
                    std::string tok;
                    int u, v, w = 1;
                    ls >> tok >> u >> v;
                    if (ls >> w)
                    { /* ok */ }
                    edges.push_back({u, v, w});
                }
                else if (!alg.empty() && V >= 0 && E >= 0 && (int)edges.size() < E) 
                {
                    // Accept bare edge lines: "u v [w]"
                    std::istringstream ls(line);
                    int u, v, w = 1;
                    if (ls >> u >> v) 
                    {
                        if (ls >> w)
                        {/* ok */}
                        edges.push_back({u, v, w});
                    }
                }
                else if (line.rfind("PARAM ", 0) == 0) 
                {
                    std::istringstream ls(line);
                    std::string tok, key;
                    int val;
                    ls >> tok >> key >> val;
                    if (key == "SRC")
                    {
                        src = val;
                    } 
                    else if (key == "SINK") 
                    {
                        sink = val;
                    }
                    else if (key == "K")
                    {
                        k = val;
                    }
                }
                else if (line.empty()) 
                {
                    continue;
                }
                else
                {
                    // Strict mode: reject on unknown directive
                    parse_error = true;
                    parse_error_msg = std::string("Unknown directive: ") + line;
                    break;
                }
            }

            if (parse_error) 
            {
                send_response(fd, parse_error_msg, false);
                continue;
            }

            if (V <= 0) 
            {
                send_response(fd, "Missing/invalid V", false);
                continue; 
            }
            if (alg == "MAX_FLOW" && src >= 0 && sink >= 0 && src == sink) 
            {
                send_response(fd, "SRC and SINK must be different", false);
                continue;
            }
            try 
            {
                Graph g(V, directed != 0);
                for (const auto &e : edges)
                {
                    g.addEdge(e.u, e.v, e.w);
                } 

                auto algoPtr = AlgorithmFactory::create(alg);
                if (!algoPtr) 
                {
                    send_response(fd, "Unsupported ALG", false);
                    continue; 
                }

                std::unordered_map<std::string,int> params;
                if (src >= 0)
                {
                    params["SRC"] = src;
                } 
                if (sink >= 0)
                {
                   params["SINK"] = sink; 
                } 
                if (k >= 0)
                {
                   params["K"] = k; 
                } 

                std::string out = algoPtr->run(g, params);
                send_response(fd, out, true);
            }
            catch (const std::exception &ex) 
            {
                send_response(fd, std::string("Exception: ") + ex.what(), false);
            }
        }
    }

    close(srv);
    return 0;
}

//Using that for handling kill/Ctrl+C->for closing the server socket:
void handle_signal(int) {
    std::cout << "\n[Signal] Shutting down server..." << std::endl;
    if (g_server_fd >= 0) close(g_server_fd);
    exit(0);   
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT,  handle_signal);  // Ctrl+C
    std::signal(SIGTERM, handle_signal);  // kill command
    return run_server(argc, argv);
}

