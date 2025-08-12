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
        TimePassedInSecondsRole,
        CycleIdRole,    ///< The cycle ID (-1 for individual regimes)
        IsCycleRole,    ///< Boolean indicating if this regime is part of a cycle
        ConditionTimeRole,      ///< Condition time in seconds
        RegimeExecutionTimeRole, ///< Pure regime execution time (without condition)
        CurrentRepeatRole,      ///< Current repeat number (0-based)
        ConditionCompletedRole, ///< Whether condition phase is completed
        ConditionTimePassedRole, ///< Time passed in condition phase (seconds)
        RegimeTimePassedRole,   ///< Time passed in execution phase (seconds)
        RepeatIndexRole,        ///< Which repeat this entry represents (0-based)
        RegimeIndexRole,        ///< Index in the original regime list
        IsCycleEntryRole,       ///< True if this is part of a cycle expansion
        CycleRepeatIndexRole    ///< Which cycle repeat this represents (0-based)
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRegimes(const QList<Regime> &regimes);
    
    // Function to be called by ProtoTableModel when total time changes
    Q_INVOKABLE void notifyTimelineUpdate();
    
private:
    struct RepeatEntry {
        Regime regime;          // The base regime
        int repeatIndex;        // Which repeat this represents (0-based)
        int regimeIndex;        // Index in the original regime list
        bool isCycleEntry;      // True if this is part of a cycle expansion
        int cycleRepeatIndex;   // Which cycle repeat this represents (0-based)
    };
    
    QList<RepeatEntry> m_repeatEntries;  // Expanded repeat entries
    
    void expandRegimesToRepeats(const QList<Regime> &regimes);

signals:
    void timelineUpdateRequired();

private:
    QList<Regime> m_regimes;
};