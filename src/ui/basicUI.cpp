#include "basicUI.h"
#include <unistd.h>

std::pair<std::string, std::string> MBBasicUI::getLogin()
{
    std::string username, password;
    std::cout << "Username: ";
    std::getline(std::cin, username);
    char* rawPass;
    if((rawPass = getpass("Password: ")) != NULL) {
        password = std::string(rawPass);
    }
    return std::make_pair(username, password);
}

void MBBasicUI::init()
{
    std::cout << "Connected to Membean!\n\n";
}

void MBBasicUI::loginFail()
{
    std::cout << "\nIncorrect login!\n\n";
}

void MBBasicUI::loginSuccess()
{
    std::cout << "\nLogged in!\n\n";
}

int MBBasicUI::getSessionLength()
{
    std::string rawLen;
    int ret = 0;
    bool valid = false;
    while(!valid) {
        std::cout << "Enter your session length (15 default): ";
        std::getline(std::cin, rawLen);
        if(rawLen == "") {
            // default
            return 15;
        }

        try {
            ret = std::stoi(rawLen);
        } catch(std::invalid_argument& e) {
            std::cout << sessionLengthError;
            continue;
        } catch(std::out_of_range& e) {
            std::cout << sessionLengthError;
            continue;
        }
        if(ret >= 5 && ret <= 60) {
            valid = true;
        } else {
            std::cout << "\nThat amount of time is out of range! Enter a number of minutes between 5 and 60.\n\n";
        }
    }
    return ret;
}