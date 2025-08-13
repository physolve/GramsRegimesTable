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
    // If type is empty or invalid, default to "none"
    if (c.type.isEmpty() || (c.type != "none" && c.type != "time" && c.type != "temp")) {
        c.type = "none";
    }
    c.temp = json["temp"].toDouble();
    c.time = json["time"].toInt();
    return c;
}

QJsonObject Regime::toJson() const {
    QJsonObject json;
    json["name"] = m_name;
    json["condition"] = m_condition.toJson();
    json["max_time"] = m_maxTime / 60;
    json["note"] = ""; // Add empty note to match original structure
    json["repeat"] = m_repeatCount;
    if (m_cycleId != -1) {
        QJsonObject cycleObj;
        cycleObj["id"] = m_cycleId;
        cycleObj["cycleRepeat"] = m_cycleRepeat;
        json["cycle"] = cycleObj;
    } else {
        json["cycle"] = QJsonValue();
    }
    return json;
}

Regime Regime::fromJson(const QJsonObject &json) {
    Regime r;
    r.m_name = json["name"].toString();
    r.m_condition = Condition::fromJson(json["condition"].toObject());
    r.m_repeatCount = json["repeat"].toInt();
    r.m_maxTime = json["max_time"].toInt() * 60;
    if (!json["cycle"].isNull()) {
        QJsonObject cycleObj = json["cycle"].toObject();
        r.m_cycleId = cycleObj["id"].toInt();
        r.m_cycleRepeat = cycleObj["cycleRepeat"].toInt();
    }
    return r;
}