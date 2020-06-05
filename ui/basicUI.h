#pragma once
#include "baseUI.h"

const std::string sessionLengthError = "\nInvalid input! Enter a number of minutes between 5 and 60.\n\n";

class MBBasicUI : public MBUserInterface {
    public:
        std::pair<std::string, std::string> getLogin();
        void loginSuccess();
        void loginFail();
        void init();
        int getSessionLength();
};