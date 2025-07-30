#include "regime.h"
#include <QJsonObject>

QJsonObject Condition::toJson() const {
    QJsonObject json;
    json["type"] = type;
    json["temp"] = temp;
    json["time"] = time;
    return json;
}

Condition Condition::fromJson(const QJsonObject &json) {
    Condition c;
    c.type = json["type"].toString();
    c.temp = json["temp"].toDouble();
    c.time = json["time"].toInt();
    return c;
}

QJsonObject Regime::toJson() const {
    QJsonObject json;
    json["name"] = m_name;
    json["condition"] = m_condition.toJson();
    QJsonObject repeatObj;
    repeatObj["count"] = m_repeatCount;
    json["repeat"] = repeatObj;
    json["max_time"] = m_maxTime;
    return json;
}

Regime Regime::fromJson(const QJsonObject &json) {
    Regime r;
    r.m_name = json["name"].toString();
    r.m_condition = Condition::fromJson(json["condition"].toObject());
    r.m_repeatCount = json["repeat"].toObject()["count"].toInt();
    r.m_maxTime = json["max_time"].toInt();
    return r;
}