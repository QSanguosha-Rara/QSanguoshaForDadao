#include "jsonutils.h"

QJsonValue QSanProtocol::Utils::toJsonArray(const QString &s1, const QString &s2) {
    QJsonArray val;
    val[0] = s1;
    val[1] = s2;
    return val;
}

QJsonValue QSanProtocol::Utils::toJsonArray(const QString &s1, const QString &s2, const QString &s3) {
    QJsonArray val;
    val[0] = s1;
    val[1] = s2;
    val[2] = s3;
    return val;
}

QJsonValue QSanProtocol::Utils::toJsonArray(const QString &s1, const QJsonValue &s2) {
    QJsonArray val;
    val[0] = s1;
    val[1] = s2;
    return val;
}

QJsonValue QSanProtocol::Utils::toJsonArray(const QList<int> &arg) {
    QJsonArray val;
    foreach (int i, arg)
        val.append(i);
    return val;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QList<int> &result) {
    if (!arg.isArray()) return false;
	QJsonArray ary = arg.toArray();
    for (int i = 0; i < ary.size(); i++)
        if (!ary[i].isDouble()) return false;
    for (int i = 0; i < ary.size(); i++)
        result.append(ary[i].toInt());
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, int &result) {
    if (!arg.isDouble()) return false;
    result = arg.toInt();
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, double &result) {
    if (!arg.isDouble()) return false;
	result = arg.toDouble();
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, bool &result) {
    if (!arg.isBool()) return false;
    result = arg.toBool();
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, Qt::Alignment &align) {
    if (!arg.isString()) return false;
	QString alignStr = arg.toString().toLower();
    if (alignStr.contains("left"))
        align = Qt::AlignLeft;
    else if (alignStr.contains("right"))
        align = Qt::AlignRight;
    else if (alignStr.contains("center"))
        align = Qt::AlignHCenter;

    if (alignStr.contains("top"))
        align |= Qt::AlignTop;
    else if (alignStr.contains("bottom"))
        align |= Qt::AlignBottom;
    else if (alignStr.contains("center"))
        align |= Qt::AlignVCenter;

    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QString &result) {
    if (!arg.isString()) return false;
    result = arg.toString();
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QStringList &result) {
    if (!arg.isArray()) return false;
	QJsonArray ary = arg.toArray();
    for (int i = 0; i < ary.size(); i++)
        if (!ary[i].isString()) return false;
    for (int i = 0; i < ary.size(); i++)
        result.append(ary[i].toString());
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QRect &result) {
    if (!arg.isArray()) return false;
	QJsonArray ary = arg.toArray();
	if (ary.size() != 4) return false;
    result.setLeft(ary[0].toInt());
    result.setTop(ary[1].toInt());
    result.setWidth(ary[2].toInt());
    result.setHeight(ary[3].toInt());
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QSize &result) {
    if (!arg.isArray()) return false;
	QJsonArray ary = arg.toArray();
	if (ary.size() != 2) return false;
    result.setWidth(ary[0].toInt());
    result.setHeight(ary[1].toInt());
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QPoint &result) {
    if (!arg.isArray()) return false;
	QJsonArray ary = arg.toArray();
	if (ary.size() != 2) return false;
    result.setX(ary[0].toInt());
    result.setY(ary[1].toInt());
    return true;
}

bool QSanProtocol::Utils::tryParse(const QJsonValue &arg, QColor &color) {
    if (!arg.isArray()) return false;
	QJsonArray ary = arg.toArray();
	if (ary.size() < 3) return false;
    color.setRed(ary[0].toInt());
    color.setGreen(ary[1].toInt());
    color.setBlue(ary[2].toInt());
    if (ary.size() > 3)
        color.setAlpha(ary[3].toInt());
    else
        color.setAlpha(255);
    return true;
}

