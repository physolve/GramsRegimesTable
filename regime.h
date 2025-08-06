#pragma once

#include <QString>
#include <QJsonObject>
#include <QVariant>

struct Condition {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(double temp MEMBER temp)
    Q_PROPERTY(int time MEMBER time)

public:
    QString type;
    double temp = 0.0;
    int time = 0;

    bool operator==(const Condition &other) const = default;

    QJsonObject toJson() const;
    static Condition fromJson(const QJsonObject &json);
};

Q_DECLARE_METATYPE(Condition)

class Regime {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(Condition condition MEMBER m_condition)
    Q_PROPERTY(int repeatCount MEMBER m_repeatCount)
    Q_PROPERTY(int maxTime MEMBER m_maxTime)
    Q_PROPERTY(int status MEMBER m_status)

public:
    QString m_name;
    Condition m_condition;
    int m_repeatCount = 0;
    int m_maxTime = 0;
    int m_cycleId = -1;
    int m_cycleRepeat = 1;
    int m_status = 0; // 0: Active, 1: Paused, 2: Skipped

    bool operator==(const Regime &other) const = default;

    QJsonObject toJson() const;
    static Regime fromJson(const QJsonObject &json);
};

Q_DECLARE_METATYPE(Regime)