#pragma once
#include <iostream>
#include "http.h"
#include "ui/basicUI.h"
#include <getopt.h>

const std::string mbPrefix = "membean.com";

struct login_data {
    std::string authToken;
    std::string tmpSessionID;
};

std::string extractString(const std::string& searchString, std::string preceding, size_t startLoc = 0);
login_data mbGetLoginData(http_response csrfTokenResponse, socket_pair& mbConnection, MBUserInterface& mainUI);
http_response mbGetCSRF(socket_pair& mbConnection);
http_response mbCreateTrainingSession(login_data newLogin, socket_pair& mbConnection, MBUserInterface& mainUI);