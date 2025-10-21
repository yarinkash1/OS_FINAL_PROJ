#include "client.hpp"

// Helper functions for printing a separator line
void println_rule()
{
    std::cout << "----------------------------------------\n"; 
}

/*
        Helper function for prompting an integer input (robust parsing):
        - Prompts with `msg` and reads a full line from stdin.
        - Trims surrounding whitespace.
        - Validation rules:
            * Accepts only decimal digits 0–9; uses std::isdigit via a safe unsigned-char wrapper.
            * Allows a leading '-' only if negatives are permitted by the range (minVal < 0).
        - Parses with std::stoll in try/catch to guard overflow/invalid input.
        - Checks the parsed value is within [minVal, maxVal] inclusive.
        - On any failure prints an error ("Invalid number" or "Out of range") and reprompts.
        - Returns the first valid integer.

        Notes/limits:
        - Only base-10 is accepted (no hex, no '+', no embedded spaces).
        - Extremely large values outside 64-bit range are rejected as invalid.
*/ 
int prompt_int(const std::string& msg, int minVal, int maxVal)
{
    while (true)
    {
        std::cout << msg;
        std::string s;
        std::getline(std::cin, s);

        // Trim whitespace
        auto l = s.find_first_not_of(" \t\r\n");
        if (l == std::string::npos) continue;
        auto r = s.find_last_not_of(" \t\r\n");
        s = s.substr(l, r - l + 1);

        if (s.empty())
        {
           continue; 
        } 

        // Allow leading '-' only when negatives are allowed by range
        const bool allowNegative = (minVal < 0);
        // lambda function to check if a character is a digit:
        auto is_digit = [](unsigned char c)
        {
            return std::isdigit(c) != 0; 
        };
        bool ok;
        if (allowNegative && s.size() > 1 && s[0] == '-')
        {
            ok = std::all_of(s.begin() + 1, s.end(), is_digit);
        }
        else
        {
            ok = std::all_of(s.begin(), s.end(), is_digit);
        }
        if (!ok)
        {
            std::cout << "Invalid number\n";
            continue;
        }

        // Safe parse with bounds check
        long long llv;
        try 
        {
            llv = std::stoll(s);
        }
        catch (...) // catch all exceptions
        {
            std::cout << "Invalid number\n";
            continue;
        }
        if (llv < static_cast<long long>(minVal) || llv > static_cast<long long>(maxVal))
        {
            std::cout << "Out of range\n";
            continue;
        }
        return static_cast<int>(llv);
    }
}

int run_client(int argc, char* argv[])
{
    int port = PORT; // Default port is 9090
    if (argc>=2) 
    {
        int p=std::atoi(argv[1]);
        if (p>0) port=p; 
    }

    std::cout<<"Part 9 Client (Pipeline Server)\n";
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0)
    {
        perror("socket");
        return 1; 
    }
    sockaddr_in sa{};
    sa.sin_family=AF_INET;
    sa.sin_port=htons(port);
    if(inet_pton(AF_INET,"127.0.0.1",&sa.sin_addr)<=0)
    {
        perror("inet_pton");
        close(sock);
        return 1; 
    }
    if(connect(sock,(sockaddr*)&sa,sizeof(sa))<0)
    {
        perror("connect");
        close(sock);
        return 1;
    }

    while(true)
    {
        println_rule();
        int mode = prompt_int("Choose: 1) ALL algorithms  0) Exit\n> ", 0, 1);
        if(mode==0)
        {
            const char* bye="EXIT\n";
            send(sock,bye,strlen(bye),0);
            close(sock);
            break;
        }

        int directed = prompt_int("Directed? 1=yes 0=no: ", 0, 1);
        int V = prompt_int("V (>=2): ", 2, 1000000);
        int maxE = directed ? V*(V-1) : V*(V-1)/2;
        if (maxE < 0) maxE = 0;
        int E = prompt_int("E (0.." + std::to_string(maxE) + "): ", 0, std::max(0, maxE));
        int SRC=-1,SINK=-1,K=-1;
        int WMIN = prompt_int("Min edge weight (>=1): ", 1, 1000000000);
        int WMAX = prompt_int("Max edge weight (>=Min): ", WMIN, 1000000000);
        SRC = prompt_int("SRC (for max flow, -1=skip): ", -1, V-1);
        if(SRC>=0)
        {
            do
            {
                SINK = prompt_int("SINK (!=SRC): ", 0, V-1);
                if(SINK==SRC)
                {
                    std::cout<<"SINK must differ\n";
                }
            }
            while(SINK==SRC);
        }
        else 
        {
            SINK=-1; 
        }
        K = prompt_int("K for cliques (-1=skip, >=2 otherwise): ", -1, V);

        // First, ask server to PREVIEW the random graph
        std::ostringstream req; req<<"ALG PREVIEW\n";
        req<<"DIRECTED "<<directed<<"\n";
        req<<"RANDOM 1\n"; // always random in part 8 flow
        req<<"V "<<V<<"\n";
        req<<"E "<<E<<"\n";
        int theSeed = (int)time(nullptr);
        req<<"SEED "<< theSeed <<"\n";
        req<<"WMIN "<< WMIN <<"\n";
        req<<"WMAX "<< WMAX <<"\n";
        if(SRC>=0)
        {
            req<<"PARAM SRC "<<SRC<<"\n";
        } 
        if(SINK>=0)
        {
            req<<"PARAM SINK "<<SINK<<"\n";
        } 
        if(K>=0)
        {
            req<<"PARAM K "<<K<<"\n";
        } 
        req<<"END\n";
        std::string s=req.str();
        send(sock,s.c_str(),s.size(),0);

        // read PREVIEW until END
        std::string resp;
        char buf[1024];
        while(true)
        {
            int n=recv(sock,buf,sizeof(buf),0);
            if(n<=0)
            {
                break; 
            }
            resp.append(buf,buf+n);
            if(resp.find("\nEND\n")!=std::string::npos|| resp.rfind("\nEND")==resp.size()-4)
            {
                break;
            }
        }
        println_rule(); std::cout<<resp<<std::endl;

        std::cout<<"Run ALL algorithms on this graph? 1=yes 0=no: ";
        std::string yn;
        std::getline(std::cin, yn);
        if(yn!="1")
        {
            continue;
        }

        // Now send ALL with the same parameters (same seed ⇒ same random graph)
        std::ostringstream req2;
        req2<<"ALG ALL\n";
        req2<<"DIRECTED "<<directed<<"\n";
        req2<<"RANDOM 1\n";
        req2<<"V "<<V<<"\n";
        req2<<"E "<<E<<"\n";
        req2<<"SEED "<< theSeed <<"\n";
        req2<<"WMIN "<< WMIN <<"\n";
        req2<<"WMAX "<< WMAX <<"\n";
        if(SRC>=0)
        {
            req2<<"PARAM SRC "<<SRC<<"\n";
        } 
        if(SINK>=0)
        {
            req2<<"PARAM SINK "<<SINK<<"\n";
        } 
        if(K>=0)
        {
            req2<<"PARAM K "<<K<<"\n";
        }
        req2<<"END\n";
        auto s2=req2.str();
        send(sock,s2.c_str(),s2.size(),0);

        // read ALL until END
        std::string resp2; while(true)
        {
            int n=recv(sock,buf,sizeof(buf),0);
            if(n<=0)
            {
                break; 
            } 
        resp2.append(buf,buf+n);
        if(resp2.find("\nEND\n")!=std::string::npos|| resp2.rfind("\nEND")==resp2.size()-4)
        {
            break;  
        }  
        }
        println_rule();
        std::cout<<resp2<<std::endl;
        std::cout<<"Press Enter to run another or type exit: ";
        std::string again;
        std::getline(std::cin,again);
        if(again=="exit")
        {
            const char* bye="EXIT\n";
            send(sock,bye,strlen(bye),0);
            close(sock);
            break; 
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    return run_client(argc, argv); 
}