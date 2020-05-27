#pragma once
#include <iostream>
#include <vector>

struct httpParam {
    std::string key;
    std::string value;
};

class HTTPResponse {
    
};

class HTTPRequest {
    private:
        std::vector<struct httpParam> requestBody;
        std::vector<struct httpParam> headerParams;
        void initializeHeaders();
    public:
        HTTPRequest(std::string host, std::string location, bool https, int requestType);
        std::vector<struct httpParam> getHeaders();
        void appendHeaderParam(struct httpParam headerParam);
        void setHeaders(std::vector<struct httpParam> newHeaders);
        std::vector<struct httpParam> getBodyContents();
        void appendBodyParam(struct httpParam bodyParam);
        void setContents(std::vector<struct httpParam> bodyContents);
        HTTPResponse connect();
};