#pragma once
#include <iostream>
#include <map>
#include "netUtil.h"

std::string urlEncodeParam(std::string str);

struct http_response {
    bool success;
    std::string body;
    std::map<std::string, std::string> headerParams;
    std::map<std::string, std::string> setCookies;
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