#pragma once
#include "baseUI.h"

class BasicUI : public MBUserInterface {
    public:
        std::pair<std::string, std::string> getLogin();
        void loginSuccess();
        void loginFail();
        void init();
};