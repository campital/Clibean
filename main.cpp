/*
TODO: proper logging
*/
#include "http.h"
#include "ui/basicUI.h"
#include <getopt.h>

int main(int argc, char **argv)
{
    // get the cmd line arguments
    // only runs getopt_long once since multiple args cannot be used simultaneously right now
    option validLongs[] = {
        {"help", 0, NULL, (int)'h'},
        {"gui", 0, NULL, (int)'g'},
        {"simple", 0, NULL, (int)'s'},
        {"fancy", 0, NULL, (int)'f'},
        {}
    };
    int option = getopt_long(argc, argv, "hgsf", validLongs, NULL);
    const std::string argErrorString = "Try '" + std::string(argv[0]) + " --help' for more information.\n";
    // TODO: Implement mode switching when an option is chosen to change how the content is displayed
    BasicUI basicUI;
    MBUserInterface& mainUI = basicUI;
    switch(option) {
        case -1:
            // TODO: set default option
            break;
        case '?':
            std::cerr << argErrorString;
            return 0;
            break;
        case 'h':
            std::cerr << "Usage: " << argv[0] << " [options]\nClibean is a desktop client for Membean (https://membean.com)\n\nOptions:\n"
            << "  -h, --help      Display this help menu\n"
            << "  -g, --gui       Run Clibean in GUI mode\n"
            << "  -s, --simple    Run Clibean in simple terminal mode (default)\n"
            << "  -f, --fancy     Run Clibean in fancy terminal mode (ncurses)\n";
            return 0;
            break;
        case 'g':
            // TODO
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
    csrfRequest.headerParams.insert(std::map<std::string, std::string>::value_type("X-Only-Token", "1"));
    http_response csrfTokenResponse = csrfRequest.connect(mbConnection);
    if(!csrfTokenResponse.success) {
        std::cerr << "Invalid CSRF token response!\n";
        return 0;
    }
    if(csrfTokenResponse.newSock.sockSSL != nullptr) {
        mbConnection = csrfTokenResponse.newSock;
    }

    mainUI.init();
    while(1) {
        auto login = mainUI.getLogin();
        HTTPRequest loginRequest("membean.com", "/login", false);
        loginRequest.headerParams.insert(std::map<std::string, std::string>::value_type("Accept",
            "*/*;q=0.5, text/javascript, application/javascript, application/ecmascript, application/x-ecmascript"));
        loginRequest.headerParams.insert(std::map<std::string, std::string>::value_type("Cookie", "_new_membean_session_id=" +
            csrfTokenResponse.setCookies["_new_membean_session_id"] + ";"));
        loginRequest.headerParams.insert(std::map<std::string, std::string>::value_type("X-CSRF-Token", csrfTokenResponse.body));
        loginRequest.requestBody.insert(std::map<std::string, std::string>::value_type("user[username]", login.first));
        loginRequest.requestBody.insert(std::map<std::string, std::string>::value_type("user[password]", login.second));
        http_response loginReponse = loginRequest.connect(mbConnection);
        if(!loginReponse.success) {
            std::cerr << "Invalid login response!\n";
            return 0;
        }
        if(loginReponse.newSock.sockSSL != nullptr) {
            mbConnection = loginReponse.newSock;
        }

        if(loginReponse.body.find("window.location.href = '/dashboard';") != std::string::npos) {
            mainUI.loginSuccess();
            break;
        } else {
            mainUI.loginFail();
        }
    }

    closeSocket(mbConnection);
    return 0;
}