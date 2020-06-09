#pragma once
#include <iostream>
#include "netUtil.h"

class HTTPRequest {
    private:
        std::string m_Host;
        std::string m_Location;
        bool m_isGet;
    public:
        std::map<std::string, std::string> requestBody;
        // the extraHeaderParams should only contain general headers, such as "cookie"
        // the HTTPRequest handles the "Host", "User-Agent", encoding, content-length, content-type, etc.
        std::map<std::string, std::string> extraHeaderParams;
        HTTPRequest(std::string host, std::string location, bool isGet);
        HTTPRequest(const HTTPRequest& other);
        http_response connect(socket_pair sock);
};

std::string urlEncodeParam(std::string str);