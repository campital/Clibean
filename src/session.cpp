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
        std::cout << "Question type: " << currQuestion.questionType << "\nQuestion ID: " << currQuestion.questionID << "\nAnswer: "
            << currQuestion.answer << "\n";
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
    
    // get the question type as a string
    size_t typeStart = html.rfind("div class='", qidPos);
    typeStart += sizeof("div class='") - 1;
    size_t typeEnd = html.find(' ', typeStart);
    if(typeEnd == std::string::npos || typeStart == std::string::npos)
        return qInfo;
    qInfo.questionType = html.substr(typeStart, typeEnd - typeStart);

    // get the encoded answer
    size_t answerEnd = html.find("id='google-analytics-mb'", typeStart);
    size_t answerStart = html.rfind("data-value='", answerEnd);
    if(typeEnd == std::string::npos || typeStart == std::string::npos)
        return qInfo;
    std::string encodedAnswer = extractString(html, "data-value=", nullptr, answerStart);
    if(encodedAnswer == "")
        return qInfo;

    // decode
    qInfo.answer = decodeAnswer(encodedAnswer);
    if(qInfo.answer == "")
        return qInfo;

    qInfo.success = true;
    return qInfo;
}

std::string MBTrainingSession::decodeAnswer(std::string encoded)
{
    // replace the html encoding (slow?)
    size_t index = 0;
	while(true) {
		index = encoded.find("&lt;", index);
		if (index == std::string::npos)
            break;
		encoded.replace(index, 4, "<");
		index++;
	}
	index = 0;
	while(true) {
		index = encoded.find("&gt;", index);
		if (index == std::string::npos)
            break;
		encoded.replace(index, 4, ">");
		index++;
	}

    for(char& c : encoded) {
        c = c ^ 14;
    }
    std::string decoded = base64Decode(encoded);
    if(decoded.size() < 21)
        return "";
    return decoded.substr(10, decoded.size() - 20);
}

std::string MBTrainingSession::base64Decode(std::string encoded)
{
    static const std::string baseTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    std::string result;
	int offset = 2;
	for(unsigned int i = 0; i < encoded.size(); i++) {
		if(encoded[i + 1] == '=')
			break;
		size_t curr = baseTable.find(encoded[i]);
		if(curr == std::string::npos)
            continue;
		curr <<= offset;
		size_t next = baseTable.find(encoded[i + 1]);
		if(next == std::string::npos)
            continue;
		curr |= next >> (6 - offset);
		offset = (offset + 2) % 6;
		if(offset == 0)
			offset = 6;
		if(offset == 2)
			i++;
		result.push_back((char)curr);
	}
	return result;
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