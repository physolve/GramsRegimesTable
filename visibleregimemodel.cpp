#include "visibleregimemodel.h"

VisibleRegimeModel::VisibleRegimeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int VisibleRegimeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_repeatEntries.count();
}

QVariant VisibleRegimeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_repeatEntries.count())
        return QVariant();

    const RepeatEntry &entry = m_repeatEntries.at(index.row());
    const Regime &regime = entry.regime;

    switch (role) {
    case NameRole:
        return regime.m_name;
    case MaxTimeRole:
        // Include condition time in the displayed max time
        {
            int conditionTimeInSeconds = 0;
            if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
                conditionTimeInSeconds = regime.m_condition.time * 60; // Convert minutes to seconds
            }
            return regime.m_maxTime + conditionTimeInSeconds;
        }
    case RepeatCountRole:
        // Return appropriate repeat count: cycle repeat for cycles, individual repeat for regimes
        if (regime.m_cycleId != -1) {
            return regime.m_cycleRepeat;
        }
        return regime.m_repeatCount;
    case StateRole:
        return QVariant::fromValue(regime.m_state);
    case TimePassedInSecondsRole:
        return regime.m_timePassedInSeconds;
    case CycleIdRole:
        return regime.m_cycleId;
    case IsCycleRole:
        return regime.m_cycleId != -1;
    case ConditionTimeRole:
        {
            int conditionTimeInSeconds = 0;
            if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
                conditionTimeInSeconds = regime.m_condition.time * 60; // Convert minutes to seconds
            }
            return conditionTimeInSeconds;
        }
    case RegimeExecutionTimeRole:
        return regime.m_maxTime; // Pure regime execution time without condition
    case CurrentRepeatRole:
        return regime.m_currentRepeat;
    case ConditionCompletedRole:
        // For this specific repeat entry, check if it's completed
        if (entry.repeatIndex < regime.m_currentRepeat) {
            return true; // Past repeats are completed
        } else if (entry.repeatIndex == regime.m_currentRepeat) {
            return regime.m_conditionCompleted; // Current repeat status
        }
        return false; // Future repeats not completed
    case ConditionTimePassedRole:
        // For this specific repeat entry, return appropriate progress
        if (entry.repeatIndex < regime.m_currentRepeat) {
            // Past repeats: show full condition time
            int conditionTimeInSeconds = 0;
            if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
                conditionTimeInSeconds = regime.m_condition.time * 60;
            }
            return conditionTimeInSeconds;
        } else if (entry.repeatIndex == regime.m_currentRepeat) {
            return regime.m_conditionTimePassed; // Current repeat progress
        }
        return 0; // Future repeats have no progress
    case RegimeTimePassedRole:
        // For this specific repeat entry, return appropriate progress
        if (entry.repeatIndex < regime.m_currentRepeat) {
            return regime.m_maxTime; // Past repeats: show full execution time
        } else if (entry.repeatIndex == regime.m_currentRepeat) {
            return regime.m_regimeTimePassed; // Current repeat progress
        }
        return 0; // Future repeats have no progress
    case RepeatIndexRole:
        return entry.repeatIndex;
    case RegimeIndexRole:
        return entry.regimeIndex;
    case IsCycleEntryRole:
        return entry.isCycleEntry;
    case CycleRepeatIndexRole:
        return entry.cycleRepeatIndex;
    }

    return QVariant();
}

QHash<int, QByteArray> VisibleRegimeModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { MaxTimeRole, "maxTime" },
        { RepeatCountRole, "repeatCount" },
        { StateRole, "state" },
        { TimePassedInSecondsRole, "timePassedInSeconds" },
        { CycleIdRole, "cycleId" },
        { IsCycleRole, "isCycle" },
        { ConditionTimeRole, "conditionTime" },
        { RegimeExecutionTimeRole, "regimeExecutionTime" },
        { CurrentRepeatRole, "currentRepeat" },
        { ConditionCompletedRole, "conditionCompleted" },
        { ConditionTimePassedRole, "conditionTimePassed" },
        { RegimeTimePassedRole, "regimeTimePassed" },
        { RepeatIndexRole, "repeatIndex" },
        { RegimeIndexRole, "regimeIndex" },
        { IsCycleEntryRole, "isCycleEntry" },
        { CycleRepeatIndexRole, "cycleRepeatIndex" }
    };
}

void VisibleRegimeModel::setRegimes(const QList<Regime> &regimes)
{
    beginResetModel();
    expandRegimesToRepeats(regimes);
    endResetModel();
}

void VisibleRegimeModel::expandRegimesToRepeats(const QList<Regime> &regimes)
{
    m_repeatEntries.clear();
    
    for (int regimeIndex = 0; regimeIndex < regimes.count(); ++regimeIndex) {
        const Regime &regime = regimes.at(regimeIndex);
        
        if (regime.m_cycleId != -1) {
            // This is part of a cycle - expand by cycle repeats
            for (int cycleRepeat = 0; cycleRepeat < regime.m_cycleRepeat; ++cycleRepeat) {
                for (int repeat = 0; repeat < regime.m_repeatCount; ++repeat) {
                    RepeatEntry entry;
                    entry.regime = regime;
                    entry.repeatIndex = repeat;
                    entry.regimeIndex = regimeIndex;
                    entry.isCycleEntry = true;
                    entry.cycleRepeatIndex = cycleRepeat;
                    m_repeatEntries.append(entry);
                }
            }
        } else {
            // Individual regime - expand by repeat count
            for (int repeat = 0; repeat < regime.m_repeatCount; ++repeat) {
                RepeatEntry entry;
                entry.regime = regime;
                entry.repeatIndex = repeat;
                entry.regimeIndex = regimeIndex;
                entry.isCycleEntry = false;
                entry.cycleRepeatIndex = 0;
                m_repeatEntries.append(entry);
            }
        }
    }
}

void VisibleRegimeModel::notifyTimelineUpdate()
{
    // Emit signal to notify TimeProgressBar that timeline needs update
    emit timelineUpdateRequired();
}