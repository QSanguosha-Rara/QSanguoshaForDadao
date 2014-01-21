#include "structs.h"
#include "jsonutils.h"
#include "protocol.h"

using namespace QSanProtocol::Utils;

bool CardsMoveStruct::tryParse(const QJsonValue &arg) {
    if (!arg.isArray() || arg.toArray().size() != 8) return false;
	QJsonArray ary = arg.toArray();
    if ((!ary[0].isDouble() && !ary[0].isArray()) ||
        !isIntArray(arg, 1, 2) || !isStringArray(arg, 3, 6)) return false;
    if (ary[0].isDouble()) {
        int size = ary[0].toInt();
        for (int i = 0; i < size; i++)
            card_ids.append(Card::S_UNKNOWN_CARD_ID);
    } else if (!QSanProtocol::Utils::tryParse(ary[0], card_ids))
        return false;
    from_place = (Player::Place)ary[1].toInt();
    to_place = (Player::Place)ary[2].toInt();
	from_player_name = ary[3].toString();
    to_player_name = ary[4].toString();
	from_pile_name = ary[5].toString();
	to_pile_name = ary[6].toString();
    reason.tryParse(ary[7]);
    return true;
}

QJsonValue CardsMoveStruct::toJsonValue() const{
    QJsonArray arg;
    if (open) arg[0] = toJsonArray(card_ids);
    else arg[0] = card_ids.size();
    arg[1] = (int)from_place;
    arg[2] = (int)to_place;
    arg[3] = from_player_name;
    arg[4] = to_player_name;
    arg[5] = from_pile_name;
    arg[6] = to_pile_name;
    arg[7] = reason.toJsonValue();
    return arg;
}

bool CardMoveReason::tryParse(const QJsonValue &arg) {
	QJsonArray ary = arg.toArray();
    m_reason = ary[0].toInt();
    m_playerId = ary[1].toString();
    m_skillName = ary[2].toString();
    m_eventName = ary[3].toString();
    m_targetId = ary[4].toString();
    return true; // @todo: fix this
}

QJsonValue CardMoveReason::toJsonValue() const{
    QJsonValue result;
    result[0] = m_reason;
    result[1] = m_playerId;
    result[2] = m_skillName;
    result[3] = m_eventName;
    result[4] = m_targetId;
    return result;
}

JsonValueForLUA::JsonValueForLUA(bool isarray): m_realvalue(isarray ? QJsonArray() : QJsonValue()){

}

bool JsonValueForLUA::getBoolAt(int n) const{
    if (n < 0)
        return m_realvalue.toBool();
    else
		return m_realvalue.toArray()[n].toBool();
}

int JsonValueForLUA::getNumberAt(int n) const{
    if (n < 0)
        return m_realvalue.toInt();
    else
        return m_realvalue.toArray[n].toInt();
}

QString JsonValueForLUA::getStringAt(int n) const{
    if (n < 0)
        return m_realvalue.toString();
    else
		return m_realvalue.toArray()[n].toString();
}

JsonValueForLUA JsonValueForLUA::getArrayAt(int n) const{
    JsonValueForLUA temp;
    if (n < 0)
        temp.m_realvalue = m_realvalue;
    else
		temp.m_realvalue = m_realvalue.toArray()[n];
    return temp;
}

void JsonValueForLUA::setBoolAt(int n, bool v){
    if (n < 0)
        m_realvalue = v;
    else {
		QJsonArray ary = m_realvalue.toArray();
		ary[n] = v;
		m_realvalue = ary;
	}
}

void JsonValueForLUA::setNumberAt(int n, int v){
    if (n < 0)
        m_realvalue = v;
    else {
        QJsonArray ary = m_realvalue.toArray();
		ary[n] = v;
		m_realvalue = ary;
	}
}

void JsonValueForLUA::setStringAt(int n, const QString &v){
    if (n < 0)
        m_realvalue = v;
    else {
        QJsonArray ary = m_realvalue.toArray();
		ary[n] = v;
		m_realvalue = ary;
	}
}

void JsonValueForLUA::setArrayAt(int n, const JsonValueForLUA &v){
    if (n < 0)
        m_realvalue = (QJsonValue)v;
    else {
        QJsonArray ary = m_realvalue.toArray();
		ary[n] = (QJsonValue)v;
		m_realvalue = ary;
	}
}