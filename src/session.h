#pragma once
#include <iostream>
#include <vector>
#include "http.h"
#include "ui/baseUI.h"

struct login_data {
    std::string authToken;
    std::string tmpSessionID;
};

struct question_info {
    bool success;
    std::string questionID;
    std::string questionType;
    std::string answer;
};

class MBTrainingSession {
    private:
        MBUserInterface& m_mainUI;
        login_data m_loginInfo;
        std::string m_sessionID;
        int m_questionTimer;
        HTTPRequest m_userStateRequest;
        std::string m_trainingSessionBase;
        question_info getQuestionInfo(const std::string& html, size_t qidPos);
        std::string decodeAnswer(std::string encoded);
        std::string base64Decode(std::string encoded);
    public:
        MBTrainingSession(MBUserInterface& mainUI, login_data loginInfo, std::string sessionID, std::string host);
        bool advanceQuestion(socket_pair& mbConnection);
};

std::string extractString(const std::string& searchString, std::string preceding, size_t* findPos = nullptr, size_t startLoc = 0);