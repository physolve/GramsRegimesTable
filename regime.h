#pragma once

#include <QString>
#include <QObject>

struct Condition {
    QString type;
    double temp;
    double time;
};

struct Repeat {
    int count;
};

class Regime : public QObject {
    Q_OBJECT
public:
    Regime(QObject *parent = nullptr) : QObject(parent) {}

    QString name;
    Condition condition;
    Repeat repeat;
};
