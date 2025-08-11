#pragma once

#include <QAbstractListModel>
#include "regime.h"

class VisibleRegimeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit VisibleRegimeModel(QObject *parent = nullptr);

    enum Role {
        NameRole = Qt::UserRole + 1,
        MaxTimeRole,
        RepeatCountRole,
        StateRole,
        TimePassedInSecondsRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRegimes(const QList<Regime> &regimes);
    
    // Function to be called by ProtoTableModel when total time changes
    Q_INVOKABLE void notifyTimelineUpdate();

signals:
    void timelineUpdateRequired();

private:
    QList<Regime> m_regimes;
};