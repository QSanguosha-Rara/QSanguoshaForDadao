#ifndef _JSON_UTILS_H
#define _JSON_UTILS_H

#include <QRect>
#include <QColor>
#include <QJsonValue>
#include <QJsonArray>

namespace QSanProtocol
{
    namespace Utils
    {
        QJsonValue toJsonArray(const QString &s1, const QString &s2);
        QJsonValue toJsonArray(const QString &s1, const QJsonValue &s2);
        QJsonValue toJsonArray(const QString &s1, const QString &s2, const QString &s3);
        QJsonValue toJsonArray(const QList<int> &);
        QJsonValue toJsonArray(const QList<QString> &);
        QJsonValue toJsonArray(const QStringList &);
        bool tryParse(const QJsonValue &, int &);
        bool tryParse(const QJsonValue &, double &);
        bool tryParse(const QJsonValue &, bool &);
        bool tryParse(const QJsonValue &, QList<int> &);
        bool tryParse(const QJsonValue &, QString &);
        bool tryParse(const QJsonValue &, QStringList &);
        bool tryParse(const QJsonValue &, QRect &);
        bool tryParse(const QJsonValue &arg, QSize &result);
        bool tryParse(const QJsonValue &arg, QPoint &result);
        bool tryParse(const QJsonValue &arg, QColor &result);
        bool tryParse(const QJsonValue &arg, Qt::Alignment &align);
    }
}

#endif

