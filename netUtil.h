#pragma once
#include <iostream>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <map>

struct socket_pair {
    int basicSock;
    SSL *sockSSL;
    SSL_CTX *ctx;
};

struct http_response {
    bool success;
    std::string body;
    std::map<std::string, std::string> headerParams;
    std::map<std::string, std::string> setCookies;
    int successCode;
};

class HTTPStreamReader {
    private:
        bool unChunk(const std::string& chunked);
        http_response m_Response = {};
        std::string m_currHeaders;
        std::string m_chunkedData;
        size_t m_headerReadPos = 0;
        size_t m_contentLength = 0;
        size_t m_contentRead = 0;
        size_t m_chunkSearch = 0;
        bool m_inHeaders = true;
        bool m_Chunked = false;
        bool m_firstBody = false;
    public:
        bool append(std::string val);
        http_response getResponse();
};

void closeSocket(socket_pair pair);
SSL* secureSocket(int normalSock, SSL_CTX** ctx);
socket_pair sslConnect(std::string hostName);
http_response writeDataSSL(SSL* ssl, std::string data);