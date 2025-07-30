#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

struct Condition {
    QString type;
    double temp = 0.0;
    double time = 0.0;

    Condition() = default;
    Condition(const QJsonObject &json) {
        type = json["type"].toString();
        temp = json["temp"].toDouble();
        time = json["time"].toDouble();
    }

    QJsonObject toJson() const {
        QJsonObject json;
        json["type"] = type;
        json["temp"] = temp;
        json["time"] = time;
        return json;
    }
};

struct Repeat {
    int count = 0;

    Repeat() = default;
    Repeat(const QJsonObject &json) {
        count = json["count"].toInt();
    }

    QJsonObject toJson() const {
        QJsonObject json;
        json["count"] = count;
        return json;
    }
};

class Regime {
public:
    QString name;
    Condition condition;
    Repeat repeat;

    Regime() = default;
    Regime(const QJsonObject &json) {
        name = json["name"].toString();
        condition = Condition(json["condition"].toObject());
        repeat = Repeat(json["repeat"].toObject());
    }

    QJsonObject toJson() const {
        QJsonObject json;
        json["name"] = name;
        json["condition"] = condition.toJson();
        json["repeat"] = repeat.toJson();
        return json;
    }
};
