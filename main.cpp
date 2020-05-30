#include "http.h"
#include <cstring>
#include <openssl/err.h>
#include <unistd.h>
#include <getopt.h>

// for memory freeing later
SSL_CTX *ctx;

void closeSocket(socket_pair pair)
{
    if(pair.sockSSL != nullptr) {
        SSL_shutdown(pair.sockSSL);
        SSL_free(pair.sockSSL);
        SSL_CTX_free(ctx);
    }
    if(pair.basicSock != -1) {
        close(pair.basicSock);
    }
}

SSL* secureSocket(int normalSock)
{
    ctx = SSL_CTX_new(TLS_client_method());
    SSL* result = SSL_new(ctx);
    if(result == NULL) {
        std::cerr << "Could not create SSL object!\n";
        return nullptr;
    }
    // connect to socket
    int fdRet = SSL_set_fd(result, normalSock);
    ERR_clear_error();
    int connectRet = SSL_connect(result);
    if(connectRet != 1) {
        std::cerr << "Could not perform TLS handshake! " << ERR_error_string(ERR_get_error(), NULL) << "\n";
        if(fdRet != 1) { std::cerr << "Could not secure socket!\n"; }
        return nullptr;
    }
    return result;
}

// returns the file descriptor for an SSL socket and the normal socket
// returns null/-1 for whatever socket(s) could not be created
socket_pair sslConnect(std::string hostName)
{
    int base;
    socket_pair pair = {-1, nullptr};
    if((base = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
        std::cerr << "Could not create socket: " << hostName << '\n';
        return pair;
    }
    // get the information to connect
    addrinfo *result;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if(getaddrinfo(hostName.c_str(), "https", &hints, &result) != 0) {
        std::cerr << "Could not get host info: " << hostName << '\n';
        return pair;
    }
    // connect to the server
    if(connect(base, result->ai_addr, result->ai_addrlen) != 0) {
        std::cerr << "Could not connect to server! Error: " << std::strerror(errno) << '\n';
        freeaddrinfo(result);
        return pair;
    }
    pair.basicSock = base;
    freeaddrinfo(result);

    pair.sockSSL = secureSocket(base);
    return pair;
}

int main(int argc, char **argv)
{
    // get the cmd line arguments
    // only runs getopt_long once since multiple args cannot be used simultaneously right now
    option validLongs[] = {
        {"help", 0, NULL, (int)'h'},
        {"gui", 0, NULL, (int)'g'},
        {"simple", 0, NULL, (int)'s'},
        {"fancy", 0, NULL, (int)'f'},
        {}
    };
    char option = (char)getopt_long(argc, argv, "hgsf", validLongs, NULL);
    const std::string argErrorString = "Try '" + std::string(argv[0]) + " --help' for more information.\n";
    // TODO: Implement mode switching when an option is chosen to change how the content is displayed
    switch(option) {
        case -1:
            // TODO: set default option
            break;
        case '?':
            std::cerr << argErrorString;
            return 0;
            break;
        case 'h':
            std::cerr << "Usage: " << argv[0] << " [options]\nClibean is a desktop client for Membean (https://membean.com)\n\n"
            << "  -h, --help      Display this help menu\n"
            << "  -g, --gui       Run Clibean in GUI mode\n"
            << "  -s, --simple    Run Clibean in simple terminal mode (default)\n"
            << "  -f, --fancy     Run Clibean in fancy terminal mode (ncurses)\n";
            return 0;
            break;
        case 'g':
            // TODO
            break;
         case 's':
            // TODO
            break;
         case 'f':
            // TODO
            break;
        default:
            std::cerr << argv[0] << ": Invalid usage!\n" << argErrorString;
            return 0;
            break;
    }

    socket_pair mbConnection = sslConnect("membean.com");
    closeSocket(mbConnection);
    return 0;
}