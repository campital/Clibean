/*
TODO: proper logging
TODO: fix sslConnect runtime errors
*/
#include "main.h"

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

    socket_pair mbConnection = sslConnect(mbPrefix);
    http_response csrfToken = mbGetCSRF(mbConnection);
    if(csrfToken.success == false) {
        return 0;
    }

    // get the login
    mainUI.init();
    login_data loginData = mbGetLoginData(csrfToken, mbConnection, mainUI);
    if(loginData.authToken == "" || loginData.tmpSessionID == "") {
        return 0;
    }

    // start a session
    http_response trainingSessionResponse = mbCreateTrainingSession(loginData, mbConnection, mainUI);
    if(trainingSessionResponse.success == false) {
        return 0;
    }
    std::string session = extractString(trainingSessionResponse.headerParams["location"], "/training_sessions");
    if(session == "") {
        std::cerr << "Error obtaining a session id!\n";
        return 0;
    }
    for(char c : session) {
        if(!(c >= '0' && c <= '9')) {
            std::cerr << "Invalid session id!\n";
            return 0;
        }
    }
    
    // now start the session
    MBTrainingSession trainer(mainUI, loginData, session, mbPrefix);
    if(!trainer.advanceQuestion(mbConnection)) {
        std::cerr << "Invalid user state response!\n";
        return 0;
    }

    closeSocket(mbConnection, false);
    return 0;
}

http_response mbGetCSRF(socket_pair& mbConnection)
{
    HTTPRequest csrfRequest(mbPrefix, "/login", true);
    // get the XSRF token and _new_membean_session_id (needed for logging in)
    csrfRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("X-Only-Token", "1"));
    http_response csrfTokenResponse = csrfRequest.connect(mbConnection);
    if(!csrfTokenResponse.success) {
        std::cerr << "Invalid CSRF token response!\n";
        return csrfTokenResponse;
    }
    if(csrfTokenResponse.newSock.sockSSL != nullptr) {
        mbConnection = csrfTokenResponse.newSock;
    }
    return csrfTokenResponse;
}

login_data mbGetLoginData(http_response csrfTokenResponse, socket_pair& mbConnection, MBUserInterface& mainUI)
{
    HTTPRequest loginRequest(mbPrefix, "/login", false);
    loginRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Accept",
        "*/*;q=0.5, text/javascript, application/javascript, application/ecmascript, application/x-ecmascript"));
    loginRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Cookie", "_new_membean_session_id=" +
        csrfTokenResponse.setCookies["_new_membean_session_id"] + ";"));
    loginRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("X-CSRF-Token", csrfTokenResponse.body));

    login_data result;
    while(1) {
        auto login = mainUI.getLogin();
        loginRequest.requestBody["user[username]"] = login.first;
        loginRequest.requestBody["user[password]"] = login.second;
        http_response loginResponse = loginRequest.connect(mbConnection);
        if(!loginResponse.success) {
            std::cerr << "Invalid login response!\n";
            return result;
        }
        if(loginResponse.newSock.sockSSL != nullptr) {
            mbConnection = loginResponse.newSock;
        }

        if(loginResponse.setCookies.find("auth_token") != loginResponse.setCookies.end()) {
            mainUI.loginSuccess();
            result.authToken = loginResponse.setCookies["auth_token"];
            result.tmpSessionID = loginResponse.setCookies["_new_membean_session_id"];
            break;
        } else {
            mainUI.loginFail();
        }
    }
    return result;
}

http_response mbCreateTrainingSession(login_data newLogin, socket_pair& mbConnection, MBUserInterface& mainUI)
{
    int sessionTime = mainUI.getSessionLength();
    HTTPRequest beginSessionRequest(mbPrefix, "/training_sessions?t=" + std::to_string(sessionTime), false);
    beginSessionRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Accept",
        "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"));
    beginSessionRequest.extraHeaderParams.insert(std::map<std::string, std::string>::value_type("Cookie", "_new_membean_session_id=" +
        newLogin.tmpSessionID + "; " + "auth_token=" + newLogin.authToken));
    http_response sessionResponse = beginSessionRequest.connect(mbConnection);
    if(sessionResponse.successCode != 302 || !sessionResponse.success) {
        std::cerr << "Invalid session creation response!\n";
        sessionResponse.success = false;
        return sessionResponse;
    }
    if(sessionResponse.newSock.sockSSL != nullptr) {
        mbConnection = sessionResponse.newSock;
    }
    return sessionResponse;
}