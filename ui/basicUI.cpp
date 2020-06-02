#include "basicUI.h"
#include "unistd.h"

std::pair<std::string, std::string> BasicUI::getLogin()
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

void BasicUI::init()
{
    std::cout << "Connected to Membean!\n\n";
}

void BasicUI::loginFail()
{
    std::cout << "\nIncorrect login!\n\n";
}

void BasicUI::loginSuccess()
{
    std::cout << "\nLogged in!\n\n";
}