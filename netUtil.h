#pragma once
#include <iostream>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <netinet/in.h>
#include <netdb.h>

struct socket_pair {
    int basicSock;
    SSL *sockSSL;
    SSL_CTX *ctx;
};

void closeSocket(socket_pair pair);
SSL* secureSocket(int normalSock, SSL_CTX** ctx);
socket_pair sslConnect(std::string hostName);
std::string writeDataSSL(SSL* ssl, std::string data);