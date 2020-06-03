/*
TODO: proper logging
TODO: delegate stuff to other places
*/
#include "http.h"
#include "ui/basicUI.h"
#include <getopt.h>

std::string extractString(const std::string& searchString, std::string preceding, size_t startLoc = 0);

int main(int argc, char **argv)
{
    // get the cmd line arguments
    // only runs getopt_long once since multiple args cannot be used simultaneously right now
    option validLongs[] = {
        {"help", 0, NULL, (int)'h'},
        {"simple", 0, NULL, (int)'s'},
        {"fancy", 0, NULL, (int)'f'},
        {}
    };
    int option = getopt_long(argc, argv, "hsf", validLongs, NULL);
    const std::string argErrorString = "Try '" + std::string(argv[0]) + " --help' for more information.\n";
    // TODO: Implement mode switching when an option is chosen to change how the content is displayed
    MBBasicUI basicUI;
    MBUserInterface& mainUI = basicUI;
    switch(option) {
        case -1:
            break;
        case '?':
            std::cerr << argErrorString;
            return 0;
            break;
        case 'h':
            std::cerr << "Usage: " << argv[0] << " [options]\nClibean is a desktop client for Membean (https://membean.com)\n\nOptions:\n"
            << "  -h, --help      Display this help menu\n"
            << "  -s, --simple    Run Clibean in simple terminal mode (default)\n"
            << "  -f, --fancy     Run Clibean in fancy terminal mode (ncurses)\n";
            return 0;
            break;
         case 's':
            // do nothing
            break;
         case 'f':
            // TODO
            return 0;
            break;
        default:
            std::cerr << argv[0] << ": Invalid usage!\n" << argErrorString;
            return 0;
            break;
    }

    socket_pair mbConnection = sslConnect("membean.com");

    HTTPRequest csrfRequest("membean.com", "/login", true);
    // get the XSRF token and _new_membean_session_id (needed for logging in)
    csrfRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("X-Only-Token", "1"));
    http_response csrfTokenResponse = csrfRequest.connect(mbConnection);
    if(!csrfTokenResponse.success) {
        std::cerr << "Invalid CSRF token response!\n";
        return 0;
    }
    if(csrfTokenResponse.newSock.sockSSL != nullptr) {
        mbConnection = csrfTokenResponse.newSock;
    }

    // get the login
    mainUI.init();
    HTTPRequest loginRequest("membean.com", "/login", false);
    loginRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Accept",
        "*/*;q=0.5, text/javascript, application/javascript, application/ecmascript, application/x-ecmascript"));
    loginRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Cookie", "_new_membean_session_id=" +
        csrfTokenResponse.setCookies["_new_membean_session_id"] + ";"));
    loginRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("X-CSRF-Token", csrfTokenResponse.body));

    std::string authToken;
    std::string sessionID;
    while(1) {
        auto login = mainUI.getLogin();
        loginRequest.requestBody["user[username]"] = login.first;
        loginRequest.requestBody["user[password]"] = login.second;
        http_response loginResponse = loginRequest.connect(mbConnection);
        if(!loginResponse.success) {
            std::cerr << "Invalid login response!\n";
            return 0;
        }
        if(loginResponse.newSock.sockSSL != nullptr) {
            mbConnection = loginResponse.newSock;
        }

        if(loginResponse.setCookies.find("auth_token") != loginResponse.setCookies.end()) {
            mainUI.loginSuccess();
            authToken = loginResponse.setCookies["auth_token"];
            sessionID = loginResponse.setCookies["_new_membean_session_id"];
            break;
        } else {
            mainUI.loginFail();
        }
    }

    // start a session
    int sessionTime = mainUI.getSessionLength();
    HTTPRequest beginSessionRequest("membean.com", "/training_sessions?t=" + std::to_string(sessionTime), false);
    beginSessionRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Accept",
        "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"));
    beginSessionRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Cookie", "_new_membean_session_id=" +
        sessionID + "; " + "auth_token=" + authToken));
    http_response sessionResponse = beginSessionRequest.connect(mbConnection);
    if(sessionResponse.successCode != 302 || !sessionResponse.success) {
        std::cerr << "Invalid session creation response!\n";
        return 0;
    }
    if(sessionResponse.newSock.sockSSL != nullptr) {
        mbConnection = sessionResponse.newSock;
    }

    //int session = std::stoi(extractString(sessionResponse.headerParams["location"], "/training_sessions"));
    std::cout << extractString(sessionResponse.headerParams["location"], "/training_sessions") << std::endl;

    closeSocket(mbConnection);
    return 0;
}

// returns empty string on failure
std::string extractString(const std::string& searchString, std::string preceding, size_t startLoc)
{
    size_t strLoc = searchString.find(preceding, startLoc);
    if(strLoc == std::string::npos) {
        return "";
    }
    strLoc += preceding.size();
    if(strLoc >= searchString.size()) {
        return "";
    }

    char endQuote = searchString[strLoc];

    strLoc++;
    size_t strEnd = strLoc;
    while((strEnd = searchString.find(endQuote, strEnd)) != std::string::npos) {
        if(searchString[strEnd - 1] != '\\') {
            return searchString.substr(strLoc, strEnd - strLoc);
        }
    }
    return "";
}