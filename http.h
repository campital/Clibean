#pragma once
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <netinet/in.h>
#include <netdb.h>

struct socket_pair {
    int basicSock;
    SSL *sockSSL;
};

struct http_response {
    bool connected;
    std::string body;
    std::unordered_map<std::string, std::string> headerParams;
    int successCode;
};

class HTTPRequest {
    private:
        std::string host;
        std::string location;
        bool isGet;
        bool ssl;
    public:
        std::unordered_map<std::string, std::string> requestBody;
        std::unordered_map<std::string, std::string> headerParams;
        HTTPRequest(std::string host, std::string location, bool isGet, bool ssl);
        http_response connect(socket_pair sock);
};