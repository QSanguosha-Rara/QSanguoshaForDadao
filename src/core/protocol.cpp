#include "protocol.h"

using namespace QSanProtocol;

unsigned int QSanProtocol::QSanGeneralPacket::_m_globalSerial = 0;
const unsigned int QSanProtocol::QSanGeneralPacket::S_MAX_PACKET_SIZE = 65535;
const QString QSanProtocol::Countdown::S_COUNTDOWN_MAGIC = "MG_COUNTDOWN";
const QString QSanProtocol::S_PLAYER_SELF_REFERENCE_ID = "MG_SELF";

bool QSanProtocol::Countdown::tryParse(QJsonValue val) {
    if (!val.isArray()) return false; 
	QJsonArray ary = val.toArray();
	if ((ary.size() != 2 && ary.size() != 3) ||
        !ary[0].isString() || ary[0].toString() != S_COUNTDOWN_MAGIC)
        return false;
    if (ary.size() == 3) {
        if (!Utils::isIntArray(val, 1, 2)) return false;
        m_current = (time_t)ary[1].toInt();
        m_max = (time_t)ary[2].toInt();
        m_type = S_COUNTDOWN_USE_SPECIFIED;
        return true;
    } else if (ary.size() == 2) {
        CountdownType type = (CountdownType)ary[1].toInt();
        if (type != S_COUNTDOWN_NO_LIMIT && type != S_COUNTDOWN_USE_DEFAULT)
            return false;
        else m_type = type;
        return true;
    } else
        return false;
}

bool QSanProtocol::Utils::isStringArray(const QJsonValue &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray()) return false;
	QJsonArray ary = jsonObject.toArray();
	if (ary.size() <= endIndex) return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!ary[i].isString())
            return false;
    }
    return true;
}

bool QSanProtocol::Utils::isIntArray(const QJsonValue &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray()) return false;
	QJsonArray ary = jsonObject.toArray();
	if (ary.size() <= endIndex) return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!ary[i].isDouble())
            return false;
    }
    return true;
}

bool QSanProtocol::QSanGeneralPacket::tryParse(const QString &s, int &val) {
    val = s.toInt();
    return true;
}

bool QSanProtocol::QSanGeneralPacket::parse(const QByteArray &bytearray) {
	QJsonDocument jd = QJsonDocument::fromJson(bytearray);
	if (!jd.isArray()) return false;
	QJsonArray result = jd.array();
    if (!Utils::isIntArray(result, 0, 3) || result.size() > 5)
        return false;

    m_globalSerial = (unsigned int)result[0].toInt();
    m_localSerial = (unsigned int)result[1].toInt();
    m_packetDescription = static_cast<PacketDescription>(result[2].toInt());
    m_command = (CommandType)result[3].toInt();

    if (result.size() == 5)
        parseBody(result[4]);
    return true;
}

QByteArray QSanProtocol::QSanGeneralPacket::toByteArray() const{
    QJsonArray result;
    result[0] = m_globalSerial;
    result[1] = m_localSerial;
    result[2] = m_packetDescription;
    result[3] = m_command;
    const QJsonValue &body = constructBody();
	if (!body.isNull())
        result[4] = body;

    return QJsonDocument(result).toJson();
}

