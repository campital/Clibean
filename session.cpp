#include "session.h"

MBTrainingSession::MBTrainingSession(MBUserInterface& mainUI, login_data loginInfo, std::string sessionID, std::string host) : m_mainUI(mainUI),
    m_loginInfo(loginInfo), m_sessionID(sessionID), m_userStateRequest(host, "/training_sessions/" + sessionID + "/user_state?xhr=_xhr", true)
{
    m_trainingSessionBase = "/training_sessions/" + sessionID;
    m_userStateRequest.extraHeaderParams["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    m_userStateRequest.extraHeaderParams["Cookie"] = "_new_membean_session_id=" + loginInfo.tmpSessionID + "; auth_token=" + loginInfo.authToken + 
        "; answered-incorrecty=";
}

bool MBTrainingSession::advanceQuestion(socket_pair& mbConnection)
{
    http_response currentState = m_userStateRequest.connect(mbConnection);
    if(!currentState.success || (currentState.successCode != 200 && currentState.successCode != 302))
        return false;
    size_t qidPos = currentState.body.find("data-qid=");
    if(qidPos != std::string::npos) {
        question_info currQuestion = getQuestionInfo(currentState.body, qidPos);
        if(!currQuestion.success)
            return false;
        std::cout << "Question type: " << currQuestion.questionType << "\nQuestion ID: " << currQuestion.questionID << "\n";
    }
    return true;
}

question_info MBTrainingSession::getQuestionInfo(const std::string& html, size_t qidPos)
{
    question_info qInfo;
    qInfo.success = false;
    std::string qid = extractString(html, "data-qid=", nullptr, qidPos);
    if(qid == "")
        return qInfo;
    qInfo.questionID = qid;
    
    size_t typeStart = html.rfind("div class='", qidPos);
    typeStart += sizeof("div class='") - 1;
    size_t typeEnd = html.find(' ', typeStart);
    if(typeEnd == std::string::npos || typeStart == std::string::npos)
        return qInfo;
    qInfo.questionType = html.substr(typeStart, typeEnd - typeStart);

    qInfo.success = true;
    return qInfo;
}

// returns empty string on failure
std::string extractString(const std::string& searchString, std::string preceding, size_t* findPos, size_t startLoc)
{
    size_t strLoc = searchString.find(preceding, startLoc);
    if(strLoc == std::string::npos)
        return "";

    if(findPos != nullptr)
        *findPos = strLoc;
    strLoc += preceding.size();
    if(strLoc >= searchString.size())
        return "";

    char endQuote = searchString[strLoc];

    strLoc++;
    size_t strEnd = strLoc;
    while((strEnd = searchString.find(endQuote, strEnd)) != std::string::npos) {
        if(searchString[strEnd - 1] != '\\') {
            return searchString.substr(strLoc, strEnd - strLoc);
        }
    }
    return "";
}