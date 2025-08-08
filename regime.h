#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QVariant>

namespace RegimeEnums {
    Q_NAMESPACE
    enum class State {
        Waiting,
        Stopped,
        Running,
        Paused,
        Skipped,
        Done,
        Error
    };
    Q_ENUM_NS(State)
}

struct Condition {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(double temp MEMBER temp)
    Q_PROPERTY(int time MEMBER time)

public:
    QString type;
    double temp = 0.0;
    // Time in minutes
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
    Q_PROPERTY(RegimeEnums::State state MEMBER m_state)

public:
    QString m_name;
    Condition m_condition;
    int m_repeatCount = 0;
    // Maximum time for the regime in seconds
    int m_maxTime = 0;
    int m_cycleId = -1;
    int m_cycleRepeat = 1;
    RegimeEnums::State m_state = RegimeEnums::State::Waiting;
    // Time passed in seconds
    int m_timePassedInSeconds = 0;
    int m_repeatsDone = 0;
    int m_repeatsSkipped = 0;
    int m_repeatsError = 0;

    bool operator==(const Regime &other) const = default;

    QJsonObject toJson() const;
    static Regime fromJson(const QJsonObject &json);
};

Q_DECLARE_METATYPE(Regime)
Q_DECLARE_METATYPE(RegimeEnums::State)
