#include "protocol.h"
#include <json/json.h>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace QSanProtocol;

unsigned int QSanProtocol::QSanGeneralPacket::_m_globalSerial = 0;
const unsigned int QSanProtocol::QSanGeneralPacket::S_MAX_PACKET_SIZE = 65535;
const string QSanProtocol::Countdown::S_COUNTDOWN_MAGIC = "MG_COUNTDOWN";
const char *QSanProtocol::S_PLAYER_SELF_REFERENCE_ID = "MG_SELF";

bool QSanProtocol::Countdown::tryParse(Json::Value val) {
    if (!val.isArray() || (val.size() != 2 && val.size() != 3) ||
        !val[0].isString() || val[0].asString() != S_COUNTDOWN_MAGIC)
        return false;
    if (val.size() == 3) {
        if (!Utils::isIntArray(val, 1, 2)) return false;
        m_current = (time_t)val[1].asInt();
        m_max = (time_t)val[2].asInt();
        m_type = S_COUNTDOWN_USE_SPECIFIED;
        return true;
    } else if (val.size() == 2) {
        CountdownType type = (CountdownType)val[1].asInt();
        if (type != S_COUNTDOWN_NO_LIMIT && type != S_COUNTDOWN_USE_DEFAULT)
            return false;
        else m_type = type;
        return true;
    } else
        return false;
}

<<<<<<< HEAD
bool QSanProtocol::Utils::isStringArray(const QJsonValue &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray()) return false;
	QJsonArray ary = jsonObject.toArray();
	if (ary.size() <= endIndex) return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!ary[i].isString())
=======
bool QSanProtocol::Utils::isStringArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray() || jsonObject.size() <= endIndex)
        return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!jsonObject[i].isString())
>>>>>>> parent of 02c29d0... a lot of protocol changes(Part 1)
            return false;
    }
    return true;
}

bool QSanProtocol::Utils::isIntArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray() || jsonObject.size() <= endIndex)
        return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!jsonObject[i].isInt())
            return false;
    }
    return true;
}

bool QSanProtocol::QSanGeneralPacket::tryParse(const string &s, int &val) {
    istringstream iss(s);
    iss >> val;
    return true;
}

<<<<<<< HEAD
<<<<<<< HEAD
bool QSanProtocol::QSanGeneralPacket::parse(const QString &string) {
	QJsonDocument jd = QJsonDocument::fromJson(string.toUtf8().toBase64());
=======
bool QSanProtocol::QSanGeneralPacket::parse(const QByteArray &bytearray) {
	QJsonDocument jd = QJsonDocument::fromJson(bytearray);
>>>>>>> parent of e32e5d9... A lot of Protocol changes(Part 2)
	if (!jd.isArray()) return false;
	QJsonArray result = jd.array();
    if (!Utils::isIntArray(result, 0, 3) || result.size() > 5)
=======
bool QSanProtocol::QSanGeneralPacket::parse(const string &s) {
    if (s.length() > S_MAX_PACKET_SIZE) {
        return false;
    }

    Json::Value result;
    bool success = m_jsonReader.parse(s, result);
    if (!success || !Utils::isIntArray(result, 0, 3) || result.size() > 5)
>>>>>>> parent of 02c29d0... a lot of protocol changes(Part 1)
        return false;

    m_globalSerial = (unsigned int)result[0].asInt();
    m_localSerial = (unsigned int)result[1].asInt();
    m_packetDescription = static_cast<PacketDescription>(result[2].asInt());
    m_command = (CommandType)result[3].asInt();

    if (result.size() == 5)
        parseBody(result[4]);
    return true;
}

<<<<<<< HEAD
<<<<<<< HEAD
QString QSanProtocol::QSanGeneralPacket::toString() const{
=======
QByteArray QSanProtocol::QSanGeneralPacket::toByteArray() const{
>>>>>>> parent of e32e5d9... A lot of Protocol changes(Part 2)
    QJsonArray result;
=======
string QSanProtocol::QSanGeneralPacket::toString() const{
    Json::Value result(Json::arrayValue);
>>>>>>> parent of 02c29d0... a lot of protocol changes(Part 1)
    result[0] = m_globalSerial;
    result[1] = m_localSerial;
    result[2] = m_packetDescription;
    result[3] = m_command;
    const Json::Value &body = constructBody();
    if (body != Json::nullValue)
        result[4] = body;

<<<<<<< HEAD
<<<<<<< HEAD
    return QString::fromUtf8(QByteArray::fromBase64(QJsonDocument(result).toJson()));
=======
    string msg = result.toStyledString();
    msg.erase(remove_if(msg.begin(), msg.end(), (int(*)(int))isspace), msg.end());
    //truncate too long messages
    if (msg.length() > S_MAX_PACKET_SIZE)
        return msg.substr(0, S_MAX_PACKET_SIZE);
    return msg;
>>>>>>> parent of 02c29d0... a lot of protocol changes(Part 1)
=======
    return QJsonDocument(result).toJson();
>>>>>>> parent of e32e5d9... A lot of Protocol changes(Part 2)
}

