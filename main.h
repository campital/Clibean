#pragma once
#include <iostream>
#include "ui/basicUI.h"
#include "session.h"
#include <getopt.h>

const std::string mbPrefix = "membean.com";

login_data mbGetLoginData(http_response csrfTokenResponse, socket_pair& mbConnection, MBUserInterface& mainUI);
http_response mbGetCSRF(socket_pair& mbConnection);
http_response mbCreateTrainingSession(login_data newLogin, socket_pair& mbConnection, MBUserInterface& mainUI);