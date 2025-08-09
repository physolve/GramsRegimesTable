#include "prototablemodel.h"

void ProtoTableModel::updateCycleIds()
{
    QMap<int, int> cycleIdMap;
    int nextCycleId = 0;

    for (int i = 0; i < m_regimes.count(); ++i) {
        if (m_regimes[i].m_cycleId != -1) {
            if (!cycleIdMap.contains(m_regimes[i].m_cycleId)) {
                cycleIdMap[m_regimes[i].m_cycleId] = nextCycleId++;
            }
            m_regimes[i].m_cycleId = cycleIdMap[m_regimes[i].m_cycleId];
        }
    }
}

ProtoTableModel::ProtoTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnNames << "Режим" << "Условие" << "Макс. время" << "Состояние";
}

int ProtoTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_regimes.count();
}

int ProtoTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_columnNames.count();
}

QVariant ProtoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_regimes.count())
        return QVariant();

    const Regime &regime = m_regimes.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return regime.m_name;
        case 1: return QVariant::fromValue(regime.m_condition);
        case 2: return regime.m_maxTime;
        case 3: return QVariant::fromValue(regime.m_state);
        default: return QVariant();
        }
    }

    if (role == RepeatRole) {
        if (regime.m_cycleId != -1) {
            return regime.m_cycleRepeat;
        }
        return regime.m_repeatCount;
    }

    if (role == MaxTimeRole) {
        return regime.m_maxTime;
    }

    if (role == ConditionRole) {
        return QVariant::fromValue(regime.m_condition);
    }

    if (role == RegimeRole) {
        return QVariant::fromValue(regime);
    }

    if (role == CycleRowCountRole) {
        if (regime.m_cycleId == -1) {
            return 1;
        }

        int span = 0;
        bool first = true;
        for (int i = 0; i < m_regimes.count(); ++i) {
            if (m_regimes.at(i).m_cycleId == regime.m_cycleId) {
                if (i < index.row()) {
                    first = false;
                    break;
                }
                span++;
            }
        }

        return first ? span : 0;
    }

    if (role == CycleStatusRole) {
        if (regime.m_cycleId == -1) {
            return 0; // Not in a cycle
        }

        bool first = true;
        for (int i = 0; i < index.row(); ++i) {
            if (m_regimes.at(i).m_cycleId == regime.m_cycleId) {
                first = false;
                break;
            }
        }

        return first ? 1 : 2; // 1 for first, 2 for subsequent
    }

    if (role == CycleRepeatRole) {
        return regime.m_cycleRepeat;
    }

    if (role == StateRole) {
        return QVariant::fromValue(regime.m_state);
    }

    if (role == TimePassedInSecondsRole) {
        return regime.m_timePassedInSeconds;
    }

    if (role == RepeatsDoneRole) {
        return regime.m_repeatsDone;
    }

    if (role == RepeatsSkippedRole) {
        return regime.m_repeatsSkipped;
    }

    if (role == RepeatsErrorRole) {
        return regime.m_repeatsError;
    }

    if (role == CycleIdRole) {
        return regime.m_cycleId;
    }

    return QVariant();
}

bool ProtoTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_regimes.count()) {
        return false;
    }
    
    Regime &regime = m_regimes[index.row()];

    if (role == RepeatRole) {
        if (regime.m_cycleId != -1) {
            for (int i = 0; i < m_regimes.count(); ++i) {
                if (m_regimes.at(i).m_cycleId == regime.m_cycleId) {
                    m_regimes[i].m_cycleRepeat = value.toInt();
                    emit dataChanged(this->index(i, 0), this->index(i, columnCount() - 1), {RepeatRole});
                }
            }
        } else {
            regime.m_repeatCount = value.toInt();
            emit dataChanged(index, index, {RepeatRole});
        }
        emit totalTimeChanged();
        return true;
    }

    if (role == CycleRepeatRole) {
        for (int i = 0; i < m_regimes.count(); ++i) {
            if (m_regimes.at(i).m_cycleId == regime.m_cycleId) {
                m_regimes[i].m_cycleRepeat = value.toInt();
                emit dataChanged(this->index(i, 0), this->index(i, columnCount() - 1), {RepeatRole});
            }
        }
        return true;
    }

    if (role == MaxTimeRole) {
        regime.m_maxTime = value.toDouble();
        emit dataChanged(index, index, {role, Qt::DisplayRole});
        emit totalTimeChanged();
        return true;
    }

    if (role == ConditionRole) {
        regime.m_condition = value.value<Condition>();
        emit dataChanged(index, index, {role, Qt::DisplayRole});
        return true;
    }

    if (role == RegimeRole) {
        m_regimes[index.row()] = value.value<Regime>();
        emit dataChanged(index, index, {role, Qt::DisplayRole});
        return true;
    }

    if (role == StateRole) {
        regime.m_state = value.value<RegimeEnums::State>();
        emit dataChanged(index, index, {role});
        checkAndUpdateRunningState();
        return true;
    }

    if (role == TimePassedInSecondsRole) {
        regime.m_timePassedInSeconds = value.toInt();
        emit dataChanged(index, index, {role});
        return true;
    }

    if (role == RepeatsDoneRole) {
        regime.m_repeatsDone = value.toInt();
        emit dataChanged(index, index, {role});
        return true;
    }

    if (role == RepeatsSkippedRole) {
        regime.m_repeatsSkipped = value.toInt();
        emit dataChanged(index, index, {role});
        return true;
    }

    if (role == RepeatsErrorRole) {
        regime.m_repeatsError = value.toInt();
        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

QVariant ProtoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return m_columnNames.at(section);
    }
    return QVariant();
}

bool ProtoTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent.isValid() || destinationParent.isValid() || sourceRow < 0 || count <= 0 || destinationChild < 0 || sourceRow + count > m_regimes.count() || destinationChild > m_regimes.count())
        return false;

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild))
        return false;

    QList<Regime> movedItems;
    for (int i = 0; i < count; ++i) {
        movedItems.append(m_regimes.takeAt(sourceRow));
    }

    int insertPos = destinationChild;
    if (sourceRow < insertPos) {
        insertPos -= count;
    }

    for (int i = 0; i < count; ++i) {
        m_regimes.insert(insertPos + i, movedItems.at(i));
    }

    endMoveRows();
    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
    emit totalTimeChanged();
    return true;
}

QHash<int, QByteArray> ProtoTableModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "display" },
        { RegimeRole, "regime" },
        { ConditionRole, "condition" },
        { RepeatRole, "repeat" },
        { MaxTimeRole, "max_time" },
        { CycleRowCountRole, "cycle_row_count" },
        { CycleStatusRole, "cycle_status" },
        { CycleRepeatRole, "cycle_repeat" },
        { StateRole, "state" },
        { TimePassedInSecondsRole, "time_passed_in_seconds" }
    };
}

Qt::ItemFlags ProtoTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void ProtoTableModel::setRegimes(const QList<Regime> &regimes)
{
    beginResetModel();
    m_regimes = regimes;
    endResetModel();
    checkAndUpdateRunningState();
}

QList<Regime> ProtoTableModel::getRegimes() const
{
    return m_regimes;
}

void ProtoTableModel::groupRows(QVariantList rows)
{
    if (rows.count() < 2) return;

    qDebug() << "groupRows called with rows:" << rows;

    int newCycleId = 0;
    for (const auto &regime : m_regimes) {
        if (regime.m_cycleId > newCycleId) {
            newCycleId = regime.m_cycleId;
        }
    }
    newCycleId++;

    qDebug() << "New cycle ID will be:" << newCycleId;
    
    for (const QVariant &rowVariant : rows) {
        int row = rowVariant.toInt();
        if (row >= 0 && row < m_regimes.count()) {
            m_regimes[row].m_cycleId = newCycleId;
            m_regimes[row].m_cycleRepeat = 1;
        } else {
            qWarning() << "groupRows: Invalid row index" << row;
        }
    }

    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1), {CycleStatusRole, CycleRowCountRole});
    emit selectionShouldBeCleared();
    emit totalTimeChanged();
}

void ProtoTableModel::ungroupRows(QVariantList rows)
{
    if (rows.isEmpty()) return;

    qDebug() << "ungroupRows called with rows:" << rows;

    QSet<int> cyclesToUngroup;
    for (const QVariant &rowVariant : rows) {
        int row = rowVariant.toInt();
        if (row >= 0 && row < m_regimes.count()) {
            if (m_regimes[row].m_cycleId != -1) {
                cyclesToUngroup.insert(m_regimes[row].m_cycleId);
            }
        } else {
            qWarning() << "ungroupRows: Invalid row index" << row;
        }
    }

    if (cyclesToUngroup.isEmpty()) return;

    qDebug() << "Cycles to ungroup:" << cyclesToUngroup;

    for (int i = 0; i < m_regimes.count(); ++i) {
        if (cyclesToUngroup.contains(m_regimes[i].m_cycleId)) {
            m_regimes[i].m_cycleId = -1;
            m_regimes[i].m_repeatCount = 1;
        }
    }

    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1), {CycleRowCountRole, RepeatRole, CycleRepeatRole, CycleStatusRole});
    emit selectionShouldBeCleared();
    emit totalTimeChanged();
}

QVariantList ProtoTableModel::moveSelection(QVariantList rows, bool up)
{
    if (rows.isEmpty())
        return QVariantList();

    int blockStart = getBlockStart(rows);
    int blockEnd = getBlockEnd(rows);
    int count = blockEnd - blockStart + 1;

    int destinationChild;
    if (up) {
        if (blockStart == 0) return rows; // Cannot move up
        destinationChild = getBlockStart({blockStart - 1});
    } else { // Moving down
        if (blockEnd == m_regimes.count() - 1) return rows; // Cannot move down
        destinationChild = getBlockEnd({blockEnd + 1}) + 1;
    }

    if (moveRows(QModelIndex(), blockStart, count, QModelIndex(), destinationChild)) {
        QVariantList newSelection;
        int newSelectionStart;
        if (up) {
            newSelectionStart = destinationChild;
        } else {
            newSelectionStart = destinationChild - count;
        }

        for (int i = 0; i < count; ++i) {
            newSelection.append(newSelectionStart + i);
        }
        return newSelection;
    }

    return rows;
}

void ProtoTableModel::addRow(const QString &regimeName)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    Regime newRegime;
    newRegime.m_name = regimeName;
    m_regimes.append(newRegime);
    endInsertRows();
    if (rowCount() > 1) {
        emit dataChanged(index(0, 0), index(rowCount() - 2, columnCount() - 1), {CycleStatusRole, CycleRowCountRole});
    }
    checkAndUpdateRunningState();
    emit totalTimeChanged();
}

void ProtoTableModel::deleteRows(QVariantList rows)
{
    if (rows.isEmpty()) return;

    QSet<int> indicesToRemoveSet;
    for (const QVariant &row : rows) {
        bool ok;
        int rowIndex = row.toInt(&ok);
        if (!ok || rowIndex < 0 || rowIndex >= m_regimes.count()) continue;

        if (m_regimes[rowIndex].m_state != RegimeEnums::State::Waiting) continue;

        if (m_regimes[rowIndex].m_cycleId != -1) {
            int cycleId = m_regimes[rowIndex].m_cycleId;
            for (int i = 0; i < m_regimes.count(); ++i) {
                if (m_regimes[i].m_cycleId == cycleId) {
                    indicesToRemoveSet.insert(i);
                }
            }
        } else {
            indicesToRemoveSet.insert(rowIndex);
        }
    }

    QList<int> sortedIndicesToRemove = indicesToRemoveSet.values();
    std::sort(sortedIndicesToRemove.begin(), sortedIndicesToRemove.end());

    if (sortedIndicesToRemove.isEmpty()) return;

    int i = sortedIndicesToRemove.count() - 1;
    while (i >= 0) {
        int lastRow = sortedIndicesToRemove[i];
        int firstRow = lastRow;
        while (i > 0 && sortedIndicesToRemove[i-1] == firstRow - 1) {
            firstRow--;
            i--;
        }

        beginRemoveRows(QModelIndex(), firstRow, lastRow);
        m_regimes.remove(firstRow, lastRow - firstRow + 1);
        endRemoveRows();

        i--;
    }

    updateCycleIds();
    if (rowCount() > 0) {
        emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1), {CycleStatusRole, CycleRowCountRole});
    }
    checkAndUpdateRunningState();
    emit totalTimeChanged();
}

void ProtoTableModel::clear()
{
    beginResetModel();
    m_regimes.clear();
    endResetModel();
    checkAndUpdateRunningState();
}

bool ProtoTableModel::isSelectionGroupable(QVariantList rows) const
{
    if (rows.count() < 2) return false;

    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex < 0 || rowIndex >= m_regimes.count()) return false;
        if (m_regimes[rowIndex].m_state != RegimeEnums::State::Waiting) return false;
    }

    int cycleCount = 0;
    int nonCycleCount = 0;

    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex >= 0 && rowIndex < m_regimes.count()) {
            if (m_regimes[rowIndex].m_cycleId != -1) {
                cycleCount++;
            } else {
                nonCycleCount++;
            }
        }
    }

    return (nonCycleCount >= 2) || (nonCycleCount >= 1 && cycleCount >=1);
}

bool ProtoTableModel::isSelectionUngroupable(QVariantList rows) const
{
    if (rows.isEmpty()) return false;

    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex < 0 || rowIndex >= m_regimes.count()) return false;
        if (m_regimes[rowIndex].m_state != RegimeEnums::State::Waiting) return false;
    }

    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex >= 0 && rowIndex < m_regimes.count()) {
            if (m_regimes[rowIndex].m_cycleId != -1) {
                return true;
            }
        }
    }

    return false;
}

bool ProtoTableModel::isMoveUpEnabled(QVariantList rows) const
{
    if (rows.isEmpty()) return false;
    int blockStart = getBlockStart(rows);
    if (blockStart == 0) return false;

    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex < 0 || rowIndex >= m_regimes.count()) return false;
        if (m_regimes[rowIndex].m_state != RegimeEnums::State::Waiting) return false;
    }

    int lastNonWaiting = -1;
    for (int i = 0; i < m_regimes.count(); ++i) {
        if (m_regimes[i].m_state != RegimeEnums::State::Waiting) {
            lastNonWaiting = i;
        }
    }

    return blockStart > lastNonWaiting + 1;
}

bool ProtoTableModel::isMoveDownEnabled(QVariantList rows) const
{
    if (rows.isEmpty()) return false;
    int blockEnd = getBlockEnd(rows);
    if (blockEnd == -1 || blockEnd == m_regimes.count() - 1) return false;

    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex < 0 || rowIndex >= m_regimes.count()) return false;
        if (m_regimes[rowIndex].m_state != RegimeEnums::State::Waiting) return false;
    }

    return true;
}

Regime ProtoTableModel::getRegime(int row) const
{
    if (row < 0 || row >= m_regimes.count()) {
        return Regime();
    }
    return m_regimes.at(row);
}

QVariantMap ProtoTableModel::getRegimeAsVariantMap(int row) const
{
    if (row < 0 || row >= m_regimes.count()) {
        return QVariantMap();
    }
    const Regime &regime = m_regimes.at(row);
    QVariantMap map;
    map.insert("name", regime.m_name);
    map.insert("condition", QVariant::fromValue(regime.m_condition));
    map.insert("repeatCount", regime.m_repeatCount);
    map.insert("maxTime", regime.m_maxTime);
    map.insert("state", QVariant::fromValue(regime.m_state));
    map.insert("timePassedInSeconds", regime.m_timePassedInSeconds);
    map.insert("repeatsDone", regime.m_repeatsDone);
    map.insert("repeatsSkipped", regime.m_repeatsSkipped);
    map.insert("repeatsError", regime.m_repeatsError);
    map.insert("cycleId", regime.m_cycleId);
    map.insert("cycleRepeat", regime.m_cycleRepeat);
    return map;
}

QVariant ProtoTableModel::get(int row, const QByteArray& roleName) const
{
    int role = roleNames().key(roleName);
    if (role == 0) { // roleName not found
        return QVariant();
    }
    return data(index(row, 0), role);
}

bool ProtoTableModel::isAnyRegimeRunning() const
{
    return m_isAnyRegimeRunning;
}

void ProtoTableModel::checkAndUpdateRunningState()
{
    bool running = false;
    for (const auto &regime : m_regimes) {
        if (regime.m_state != RegimeEnums::State::Waiting &&
            regime.m_state != RegimeEnums::State::Skipped &&
            regime.m_state != RegimeEnums::State::Done) {
            running = true;
            break;
        }
    }

    if (m_isAnyRegimeRunning != running) {
        m_isAnyRegimeRunning = running;
    }
}

int ProtoTableModel::getBlockStart(QVariantList rows) const
{
    if (rows.isEmpty()) return -1;

    QList<int> selectedIndices;
    for (const QVariant &v : rows) {
        bool ok;
        int index = v.toInt(&ok);
        if (!ok || index < 0 || index >= m_regimes.count()) {
            qWarning() << "getBlockStart: Invalid row index in list:" << v;
            return -1; // Invalid index
        }
        selectedIndices.append(index);
    }
    std::sort(selectedIndices.begin(), selectedIndices.end());

    if (selectedIndices.isEmpty()) return -1;

    int blockStart = selectedIndices.first();

    for (int i = 0; i < selectedIndices.count(); ++i) {
        int rowIndex = selectedIndices[i];
        if (m_regimes[rowIndex].m_cycleId != -1) {
            int cycleId = m_regimes[rowIndex].m_cycleId;
            int cycleStart = rowIndex;
            while (cycleStart > 0 && m_regimes[cycleStart - 1].m_cycleId == cycleId) {
                cycleStart--;
            }
            blockStart = qMin(blockStart, cycleStart);
        }
    }
    return blockStart;
}

int ProtoTableModel::getBlockEnd(QVariantList rows) const
{
    if (rows.isEmpty()) return -1;

    QList<int> selectedIndices;
    for (const QVariant &v : rows) {
        bool ok;
        int index = v.toInt(&ok);
        if (!ok || index < 0 || index >= m_regimes.count()) {
            qWarning() << "getBlockEnd: Invalid row index in list:" << v;
            return -1; // Invalid index
        }
        selectedIndices.append(index);
    }
    std::sort(selectedIndices.begin(), selectedIndices.end());

    if (selectedIndices.isEmpty()) return -1;

    int blockEnd = selectedIndices.last();

    for (int i = 0; i < selectedIndices.count(); ++i) {
        int rowIndex = selectedIndices[i];
        if (m_regimes[rowIndex].m_cycleId != -1) {
            int cycleId = m_regimes[rowIndex].m_cycleId;
            int cycleEnd = rowIndex;
            while (cycleEnd < m_regimes.count() - 1 && m_regimes[cycleEnd + 1].m_cycleId == cycleId) {
                cycleEnd++;
            }
            blockEnd = qMax(blockEnd, cycleEnd);
        }
    }
    return blockEnd;
}