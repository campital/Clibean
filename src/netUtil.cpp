#include "netUtil.h"
#include <cstring>
#include <openssl/err.h>
#include <unistd.h>
#include <iostream>
#include <memory>

const int bufSize = 2048;
const std::string validHex = "0123456789abcdefABCDEF";

void closeSocket(socket_pair pair, bool fatalError)
{
    if(pair.sockSSL != nullptr) {
        if(!fatalError) {
            auto tmpBuf = std::unique_ptr<char[]>(new char[bufSize]);
            int retStatus = 1;
            bool read = false;
            while(SSL_has_pending(pair.sockSSL)) {
                retStatus = SSL_read(pair.sockSSL, tmpBuf.get(), bufSize);
                read = true;
                if(retStatus < 1)
                    break;
            }

            int sslError = SSL_ERROR_NONE;
            if(read)
                sslError = SSL_get_error(pair.sockSSL, retStatus);
            if(sslError != SSL_ERROR_SSL && sslError != SSL_ERROR_SYSCALL)
                SSL_shutdown(pair.sockSSL);
        }
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
        throw std::runtime_error("Could not create socket: " + hostName);
        return pair;
    }
    // get the information to connect
    addrinfo *result;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if(getaddrinfo(hostName.c_str(), "https", &hints, &result) != 0) {
        throw std::runtime_error("Could not get host info: " + hostName);
        return pair;
    }

    // set timeouts
    timeval tOut;
    tOut.tv_sec = 5;
    tOut.tv_usec = 0;
    setsockopt(base, SOL_SOCKET, SO_RCVTIMEO, &tOut, sizeof(tOut));
    setsockopt(base, SOL_SOCKET, SO_SNDTIMEO, &tOut, sizeof(tOut));

    // connect to the server
    if(connect(base, result->ai_addr, result->ai_addrlen) != 0) {
        throw std::runtime_error(std::string("Could not connect to server! Error: ") + std::strerror(errno));
        freeaddrinfo(result);
        return pair;
    }
    pair.basicSock = base;
    freeaddrinfo(result);

    pair.sockSSL = secureSocket(base, &(pair.ctx));
    return pair;
}

// remove the chunkiness from Transfer-Encoding: chunked
// throws an exception if something is invalid
// returns true if it reaches the end
bool HTTPStreamReader::unChunk(const std::string& chunked)
{
    std::string result;
    m_chunkedData.append(chunked);

    while(m_chunkSearch < m_chunkedData.size()) {
        size_t endSection = m_chunkedData.find("\r\n", m_chunkSearch);
        if(endSection == std::string::npos) {
            m_Response.body += result;
            return false;
        }

        std::string hexReadLen = m_chunkedData.substr(m_chunkSearch, endSection - m_chunkSearch);
        for(char c : hexReadLen) {
            if(validHex.find(c) == std::string::npos) throw std::invalid_argument("Invalid chunked data!");
        }
        size_t readLen = std::stoul(hexReadLen, nullptr, 16);
        if(readLen == 0) {
            m_Response.body += result;
            return true;
        }
        if(endSection + readLen + 4 >= m_chunkedData.size()) {
            m_Response.body += result;
            return false;
        } else {
            m_chunkSearch = endSection + 2;
            result += m_chunkedData.substr(m_chunkSearch, readLen);
        }
        m_chunkSearch += readLen + 2;
    }
    m_Response.body += result;
    return false;
}

bool HTTPStreamReader::append(std::string val)
{
    m_currHeaders += val;
    if(m_headerReadPos == 0) {
        size_t endLine = m_currHeaders.find("\r\n");
        if(endLine == std::string::npos) {
            return false;
        }

        size_t endCode = m_currHeaders.rfind(" ", endLine);
        size_t startCode = m_currHeaders.find("HTTP/1.1");
        if(endCode != std::string::npos && startCode != std::string::npos) {
            m_Response.successCode = std::stoul(m_currHeaders.substr(startCode + 9, endCode - (startCode + 9)));
        }
        m_headerReadPos = endLine + 2;
    }
    if(m_inHeaders) {
        while(true) {
            size_t endLine = m_currHeaders.find("\r\n", m_headerReadPos);
            if(endLine == std::string::npos) {
                return false;
            }
            std::string currLine = m_currHeaders.substr(m_headerReadPos, endLine - m_headerReadPos);
            // check if the headers are over
            if(currLine == "") {
                m_inHeaders = false;
                m_firstBody = true;
                std::map<std::string, std::string>::const_iterator it;
                if((it = m_Response.headerParams.find("transfer-encoding")) != m_Response.headerParams.end()) {
                    if(it->second == "chunked") {
                        m_Chunked = true;
                    }
                } else if((it = m_Response.headerParams.find("content-length")) == m_Response.headerParams.end()) {
                    return true;
                } else {
                    m_contentLength = std::stoul(m_Response.headerParams["content-length"]);
                }
                m_headerReadPos += 2;
                break;
            } else {
                // parse the current line for headers
                size_t endLine = m_currHeaders.find("\r\n", m_headerReadPos);
                if(endLine == std::string::npos) {
                    return false;
                }
                size_t colon = m_currHeaders.find(':', m_headerReadPos);
                if(colon == std::string::npos) {
                    return false;
                }
                if(colon > endLine) {
                    throw std::invalid_argument("Invalid headers!");
                }
                std::string key = m_currHeaders.substr(m_headerReadPos, colon - m_headerReadPos);
                // lowercase it
                for(char &c : key) { c = std::tolower(c); }

                // insert the headers into the std::map(s)
                std::string val = m_currHeaders.substr(colon + 2, endLine - (colon + 2));
                if(key != "set-cookie") {
                    m_Response.headerParams.insert(std::map<std::string, std::string>::value_type(key, val));
                } else {
                    size_t separateCookie = val.find("=");
                    size_t endCookie = val.find(";");
                    if(separateCookie == std::string::npos || endCookie == std::string::npos) {
                        return false;
                    }
                    m_Response.setCookies.insert(std::map<std::string, std::string>::value_type(val.substr(0, separateCookie),
                        val.substr(separateCookie + 1, endCookie - (separateCookie + 1))));
                }
                m_headerReadPos = endLine + 2;
            }
        }
    }
    // it could have changed
    if(!m_inHeaders) {
        // extract the body
        if(m_firstBody) {
            m_firstBody = false;
            if(m_headerReadPos < m_currHeaders.size()) {
                val = m_currHeaders.substr(m_headerReadPos);
            } else {
                return false;
            }
        }
        // the data is chunked
        if(m_Chunked) {
            if(unChunk(val)) {
                return true;
            }
        } else {
            m_contentRead += val.size();
            m_Response.body.append(val);
            if(m_contentRead >= m_contentLength) {
                return true;
            }
        }
    }
    return false;
}

http_response HTTPStreamReader::getResponse()
{
    return m_Response;
}


// returns the response (success in the struct will be set appropriately)
// parses data as it receives
http_response writeDataSSL(SSL* ssl, std::string data)
{
    int retry = 0;
    while(retry < 4) {
        int err = SSL_write(ssl, data.c_str(), data.size());
        if(err > 0) {
            break;
        } else if((err = SSL_get_error(ssl, err)) != SSL_ERROR_WANT_WRITE) {
            http_response sslErr;
            sslErr.success = false;
            sslErr.sslError = err;
            return sslErr;
        }
        retry++;
    }

    auto buf = std::unique_ptr<char[]>(new char[bufSize]);
    int readSize = 0;
    HTTPStreamReader parser;
    do {
        readSize = SSL_read(ssl, buf.get(), bufSize - 1);
        if(readSize > 0) {
            buf[readSize] = 0;
            std::string currBuf(buf.get());
            if(parser.append(currBuf)) {
                http_response res = parser.getResponse();
                res.success = true;
                return res;
            }
        } else if((readSize = SSL_get_error(ssl, readSize)) != SSL_ERROR_WANT_READ) {
            http_response sslErr;
            sslErr.success = false;
            sslErr.sslError = readSize;
            return sslErr;
        }
    } while(readSize > 0);
    http_response res = parser.getResponse();
    res.success = false;
    return res;
}