#include "http.h"

const std::string allowedUrl = "_.-~";
const std::string hexTable = "0123456789ABCDEF";

std::string toHex(unsigned int i)
{
    unsigned int place = 1;
    std::string res;
    int count = 0;
    while(i != 0 || count < 2) {
        res.insert(res.begin(), hexTable[i % 16]);
        i /= (place *= 16);
        count++;
    }
    return res;
}

// this encodes an url param, not a full url (it does not allow ?&=/ even though those can exist in a url)
std::string urlEncodeParam(std::string str)
{
    std::string res;
    // assumes 4 special chars
    res.reserve(str.size() + 12); 
    for(char c : str) {
        if(isalnum(c) || allowedUrl.find(c) != std::string::npos) {
            res.push_back(c);
        } else {
            res.append("%" + toHex((unsigned int)c));
        }
    }
    return res;
}

// host is the hostname (ie "www.google.com" or "google.com")
// location is the information after the host (ie "/search" for a GET request)
HTTPRequest::HTTPRequest(std::string host, std::string location, bool isGet) :
    m_Host(host), m_Location(location), m_isGet(isGet) {}

HTTPRequest::HTTPRequest(const HTTPRequest& other)
{
    m_Host = other.m_Host;
    m_Location = other.m_Location;
    m_isGet= other.m_isGet;
    requestBody = other.requestBody;
    headerParams = other.headerParams;
}

// note that headerParams WILL BE modified, but can be requested again
// on networking failure, returns a failed http_response (should reconnect)
http_response HTTPRequest::connect(socket_pair sock)
{
    std::string headers;
    // just some arbitrary number I think would be good
    headers.reserve(512);
    auto bodyIt = requestBody.begin();
    std::string getParams;
    if(m_isGet && requestBody.size() > 0) {
        getParams.push_back('?');
        while(bodyIt != requestBody.end()) {
            getParams.append(urlEncodeParam(bodyIt->first) + "=" + urlEncodeParam(bodyIt->second) + "&");
            bodyIt++;
        }
        getParams.resize(getParams.size() - 1);
    }
    headers.append(m_isGet ? "GET" : "POST").append(" " + m_Location + getParams).append(" HTTP/1.1\r\n");
    headerParams.insert(std::map<std::string, std::string>::value_type("Host", m_Host));
    // just copied User-Agent from my browser (could be anything but don't want to trigger suspicion)
    headerParams.insert(std::map<std::string, std::string>::value_type("User-Agent",
        "Mozilla/5.0 (X11; Linux x86_64; rv:76.0) Gecko/20100101 Firefox/76.0"));
    // TODO: add language support??
    headerParams.insert(std::map<std::string, std::string>::value_type("Accept-Language", "en-US,en;q=0.5"));
    headerParams.insert(std::map<std::string, std::string>::value_type("Accept-Encoding", "identity"));
    // server just times out anyways
    headerParams.insert(std::map<std::string, std::string>::value_type("Connection", "keep-alive"));
    auto headerIt = headerParams.begin();
    while(headerIt != headerParams.end()) {
        headers.append(headerIt->first + ": " + headerIt->second + "\r\n");
        headerIt++;
    }

    // form the body of the POST
    std::string body;
    bodyIt = requestBody.begin();
    if(!m_isGet && requestBody.size() > 0) {
        body.reserve(256);
        while(bodyIt != requestBody.end()) {
            body.append(urlEncodeParam(bodyIt->first) + "=" + urlEncodeParam(bodyIt->second) + "&");
            bodyIt++;
        }
        body.resize(body.size() - 1);
        body.append("\r\n\r\n");
        headers.append("Content-Type: application/x-www-form-urlencoded\r\n");
        headers.append("Content-Length: " + std::to_string(body.size() - 4) + "\r\n");
    }
    headers.append("\r\n");
    http_response finalRes;
    socket_pair newSock;
    newSock.sockSSL = nullptr;
    // send the request (try 3 times)
    for(int i = 0; i < 3; i++) {
        bool retry = false;
        try {
            finalRes = writeDataSSL(sock.sockSSL, headers + body);
            if(!finalRes.success) {
                if(finalRes.sslError == SSL_ERROR_WANT_CONNECT || finalRes.sslError == SSL_ERROR_SSL
                    || finalRes.sslError == SSL_ERROR_SYSCALL || finalRes.sslError == SSL_ERROR_ZERO_RETURN) {
                        
                    // reconnect
                    closeSocket(sock);
                    sock = sslConnect(m_Host);
                    newSock = sock;
                }
                retry = true;
            }
        } catch(const std::invalid_argument& t) {
            std::cerr << t.what() << '\n';
            retry = true;
        }
        if(!retry) {
            break;
        }
    }
    finalRes.newSock = newSock;
    return finalRes;
}