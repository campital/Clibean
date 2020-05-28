#include "http.h"

HTTPRequest::HTTPRequest(std::string host, std::string location, bool isGet, bool ssl)
{
    this->host = host;
    this->location = location;
    this->isGet = isGet;
    this->ssl = ssl;
}

// TODO
http_response HTTPRequest::connect(socket_pair sock)
{

}