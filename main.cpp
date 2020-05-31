/*
TODO: proper logging
TODO: transfer decoding chunked
*/
#include "http.h"
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
            break;
         case 's':
            // TODO
            break;
         case 'f':
            // TODO
            break;
        default:
            std::cerr << argv[0] << ": Invalid usage!\n" << argErrorString;
            return 0;
            break;
    }

    socket_pair mbConnection = sslConnect("membean.com");

    HTTPRequest loginRequest("membean.com", "/login", true, true);
    // get the XSRF token
    loginRequest.headerParams.insert(std::map<std::string, std::string>::value_type("X-Only-Token", "1"));
    //loginRequest.headerParams.insert(std::map<std::string, std::string>::value_type("Connection", "keep-alive"));
    http_response csrfTokenResponse = loginRequest.connect(mbConnection);
    std::cout << csrfTokenResponse.setCookies["_new_membean_session_id"] << std::endl;

    closeSocket(mbConnection);
    return 0;
}