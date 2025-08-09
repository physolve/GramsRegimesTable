#include "visibleregimemodel.h"

VisibleRegimeModel::VisibleRegimeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int VisibleRegimeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_regimes.count();
}

QVariant VisibleRegimeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_regimes.count())
        return QVariant();

    const Regime &regime = m_regimes.at(index.row());

    switch (role) {
    case NameRole:
        return regime.m_name;
    case MaxTimeRole:
        return regime.m_maxTime;
    case RepeatCountRole:
        return regime.m_repeatCount;
    case StateRole:
        return QVariant::fromValue(regime.m_state);
    case TimePassedInSecondsRole:
        return regime.m_timePassedInSeconds;
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
        { TimePassedInSecondsRole, "timePassedInSeconds" }
    };
}

void VisibleRegimeModel::setRegimes(const QList<Regime> &regimes)
{
    beginResetModel();
    m_regimes = regimes;
    endResetModel();
}
