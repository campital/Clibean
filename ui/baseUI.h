#pragma once
#include <iostream>
#include <utility>

// this class is an abstract class / interface (needs an implementation)
class MBUserInterface {
    public:
        virtual std::pair<std::string, std::string> getLogin() = 0;
        virtual void init() = 0;
        virtual void loginSuccess() = 0;
        virtual void loginFail() = 0;
};