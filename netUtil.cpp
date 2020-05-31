#include "netUtil.h"
#include <cstring>
#include <openssl/err.h>
#include <unistd.h>
#include <iostream>

const int bufSize = 1024;

void closeSocket(socket_pair pair)
{
    if(pair.sockSSL != nullptr) {
        SSL_shutdown(pair.sockSSL);
        SSL_free(pair.sockSSL);
        SSL_CTX_free(pair.ctx);
    }
    if(pair.basicSock != -1) {
        close(pair.basicSock);
    }
}

SSL* secureSocket(int normalSock, SSL_CTX** ctx)
{
    *ctx = SSL_CTX_new(TLS_client_method());
    SSL* result = SSL_new(*ctx);
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

    pair.sockSSL = secureSocket(base, &(pair.ctx));
    return pair;
}

// returns the response (blank if timed out or network error)
std::string writeDataSSL(SSL* ssl, std::string data)
{
    int retry = 0;
    while(retry < 4) {
        int err = SSL_write(ssl, data.c_str(), data.size());
        if(err > 0) {
            break;
        } else if(SSL_get_error(ssl, err) != SSL_ERROR_WANT_WRITE) {
            return "";
        }
        retry++;
    }
    char* buf = new char[bufSize];
    int readSize = 0;
    std::string bufString;
    retry = 0;
    do {
        readSize = SSL_read(ssl, buf, bufSize - 1);
        if(readSize > 0 && readSize < bufSize) {
            buf[readSize] = 0;
            bufString += std::string(buf);
        }
    } while(readSize > 0);
    delete [] buf;
    return bufString;
}