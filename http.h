#pragma once
#include <iostream>
#include <map>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <netinet/in.h>
#include <netdb.h>

std::string urlEncodeParam(std::string str);

struct socket_pair {
    int basicSock;
    SSL *sockSSL;
};

struct http_response {
    bool connected;
    std::string body;
    std::map<std::string, std::string> headerParams;
    int successCode;
};

class HTTPRequest {
    private:
        std::string m_Host;
        std::string m_Location;
        bool m_isGet;
        bool m_ssl;
    public:
        std::map<std::string, std::string> requestBody;
        // the headerParams should only contain general headers, such as "cookie"
        // the HTTPRequest handles the "Host", "User-Agent", encoding, content-length, content-type, etc.
        std::map<std::string, std::string> headerParams;
        HTTPRequest(std::string host, std::string location, bool isGet, bool ssl);
        HTTPRequest(const HTTPRequest& other);
        http_response connect(socket_pair sock);
};