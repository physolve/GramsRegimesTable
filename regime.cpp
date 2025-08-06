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
    if (m_cycleId != -1) {
        QJsonObject cycleObj;
        cycleObj["id"] = m_cycleId;
        cycleObj["cycleRepeat"] = m_cycleRepeat;
        json["cycle"] = cycleObj;
    } else {
        json["cycle"] = QJsonValue();
    }
    json["status"] = m_status;
    return json;
}

Regime Regime::fromJson(const QJsonObject &json) {
    Regime r;
    r.m_name = json["name"].toString();
    r.m_condition = Condition::fromJson(json["condition"].toObject());
    r.m_repeatCount = json["repeat"].toObject()["count"].toInt();
    r.m_maxTime = json["max_time"].toInt();
    if (!json["cycle"].isNull()) {
        QJsonObject cycleObj = json["cycle"].toObject();
        r.m_cycleId = cycleObj["id"].toInt();
        r.m_cycleRepeat = cycleObj["cycleRepeat"].toInt();
    }
    r.m_status = json["status"].toInt();
    return r;
}