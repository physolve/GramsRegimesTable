#include "prototablemodel.h"

void ProtoTableModel::loadDataFromJson()
{
    QDir dir("profile");
    if(!dir.exists()) 
        return;
    QFile file;
    file.setFileName(dir.filePath("regime_a.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open regime_a.json");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonArray regimesArray = doc.array();

    beginResetModel();
    m_regimes.clear();

    for (const QJsonValue &value : regimesArray) {
        m_regimes.append(Regime::fromJson(value.toObject()));
    }

    endResetModel();
}

void ProtoTableModel::saveDataToJson()
{
    QDir dir("profile");
    if(!dir.exists()) 
        return;
    QFile file;
    file.setFileName(dir.filePath("regime_a.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file for writing");
        return;
    }

    QJsonArray regimesArray;
    for (const auto &regime : m_regimes) {
        regimesArray.append(regime.toJson());
    }

    QJsonDocument doc(regimesArray);
    file.write(doc.toJson());
    file.close();
}

ProtoTableModel::ProtoTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnNames << "Режим" << "Условие" << "Макс. время";
    loadDataFromJson();
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

    if (role == SpanRole) {
        if (regime.m_cycleId == -1) {
            return 1;
        }

        // Check if this is the first row in the cycle
        if (index.row() > 0 && m_regimes.at(index.row() - 1).m_cycleId == regime.m_cycleId) {
            return 1; // Not the first row, so span is 0
        }

        int span = 1;
        for (int i = index.row() + 1; i < m_regimes.count(); ++i) {
            if (m_regimes.at(i).m_cycleId == regime.m_cycleId) {
                span++;
            } else {
                break;
            }
        }
        return span;
    }

    if (role == CycleStatusRole) {
        if (regime.m_cycleId == -1) {
            return 0; // Not in a cycle
        }
        if (index.row() > 0 && m_regimes.at(index.row() - 1).m_cycleId == regime.m_cycleId) {
            return 2; // Subsequent in cycle
        }
        return 1; // First in cycle
    }

    if (role == CycleRepeatRole) {
        return regime.m_cycleRepeat;
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

    return false;
}

QVariant ProtoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return m_columnNames.at(section);
    }
    return QVariant();
}

QHash<int, QByteArray> ProtoTableModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "display" },
        { RegimeRole, "regime" },
        { ConditionRole, "condition" },
        { RepeatRole, "repeat" },
        { MaxTimeRole, "max_time" },
        { SpanRole, "span" },
        { CycleStatusRole, "cycle_status" },
        { CycleRepeatRole, "cycle_repeat" }
    };
}

Qt::ItemFlags ProtoTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void ProtoTableModel::updateCycleIds()
{
    int currentCycleId = 1;
    int lastSeenCycleId = -1;
    for (int i = 0; i < m_regimes.count(); ++i) {
        if (m_regimes[i].m_cycleId != -1) {
            if (m_regimes[i].m_cycleId != lastSeenCycleId) {
                lastSeenCycleId = m_regimes[i].m_cycleId;
                int newId = currentCycleId++;
                // Update all regimes with the old ID to the new one
                for (int j = i; j < m_regimes.count(); ++j) {
                    if (m_regimes[j].m_cycleId == lastSeenCycleId) {
                        m_regimes[j].m_cycleId = newId;
                    }
                }
                lastSeenCycleId = newId;
            }
        }
    }
}

void ProtoTableModel::groupRows(QVariantList rows)
{
    if (rows.count() < 2) return;

    QList<int> rowIndices;
    for (const QVariant &row : rows) {
        rowIndices.append(row.toInt());
    }

    int targetCycleId = -1;
    // Check if any of the selected rows are already in a cycle.
    for (int rowIndex : rowIndices) {
        if (m_regimes[rowIndex].m_cycleId != -1) {
            targetCycleId = m_regimes[rowIndex].m_cycleId;
            break;
        }
    }

    // If no existing cycle was found, find the max cycleId and increment it.
    if (targetCycleId == -1) {
        targetCycleId = 0;
        for (const auto &regime : m_regimes) {
            if (regime.m_cycleId > targetCycleId) {
                targetCycleId = regime.m_cycleId;
            }
        }
        targetCycleId++;
    }

    // Assign the targetCycleId to all selected rows.
    for (int rowIndex : rowIndices) {
        m_regimes[rowIndex].m_cycleId = targetCycleId;
    }

    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
    saveDataToJson();
}

void ProtoTableModel::ungroupRows(QVariantList rows)
{
    if (rows.isEmpty()) return;

    QSet<int> cyclesToUngroup;
    for (const QVariant &row : rows) {
        int rowIndex = row.toInt();
        if (rowIndex >= 0 && rowIndex < m_regimes.count()) {
            if (m_regimes[rowIndex].m_cycleId != -1) {
                cyclesToUngroup.insert(m_regimes[rowIndex].m_cycleId);
            }
        }
    }

    if (cyclesToUngroup.isEmpty()) return;

    for (int i = 0; i < m_regimes.count(); ++i) {
        if (cyclesToUngroup.contains(m_regimes[i].m_cycleId)) {
            m_regimes[i].m_cycleId = -1;
        }
    }

    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
    saveDataToJson();
}

QVariantList ProtoTableModel::moveRows(QVariantList rows, bool up)
{
    if (rows.isEmpty())
        return QVariantList();

    QList<int> selectedIndices;
    for (const QVariant &v : rows) {
        selectedIndices.append(v.toInt());
    }
    std::sort(selectedIndices.begin(), selectedIndices.end());

    // Find the full block of rows to move, expanding to contain full cycles.
    int blockStart = selectedIndices.first();
    int blockEnd = selectedIndices.last();

    for (int i = blockStart; i <= blockEnd; ++i) {
        if (m_regimes[i].m_cycleId != -1) {
            int cycleId = m_regimes[i].m_cycleId;
            int cycleStart = i;
            while (cycleStart > 0 && m_regimes[cycleStart - 1].m_cycleId == cycleId) {
                cycleStart--;
            }
            int cycleEnd = i;
            while (cycleEnd < m_regimes.count() - 1 && m_regimes[cycleEnd + 1].m_cycleId == cycleId) {
                cycleEnd++;
            }
            blockStart = qMin(blockStart, cycleStart);
            blockEnd = qMax(blockEnd, cycleEnd);
        }
    }

    int blockCount = blockEnd - blockStart + 1;
    QVariantList newSelection;

    if (up) {
        if (blockStart == 0) return rows;

        int targetBlockEnd = blockStart - 1;
        int targetCycleId = m_regimes[targetBlockEnd].m_cycleId;
        int targetBlockStart = targetBlockEnd;
        while (targetBlockStart > 0 && m_regimes[targetBlockStart - 1].m_cycleId == targetCycleId) {
            targetBlockStart--;
        }

        if (beginMoveRows(QModelIndex(), blockStart, blockEnd, QModelIndex(), targetBlockStart)) {
            QList<Regime> movedItems;
            for (int i = 0; i < blockCount; ++i) {
                movedItems.append(m_regimes.takeAt(blockStart));
            }
            for (int i = 0; i < blockCount; ++i) {
                m_regimes.insert(targetBlockStart + i, movedItems.at(i));
            }
            endMoveRows();

            for (int i = 0; i < blockCount; ++i) {
                newSelection.append(targetBlockStart + i);
            }
        }
    } else { // Moving down
        if (blockEnd == m_regimes.count() - 1) return rows;

        int targetBlockStart = blockEnd + 1;
        int targetCycleId = m_regimes[targetBlockStart].m_cycleId;
        int targetBlockEnd = targetBlockStart;
        while (targetBlockEnd < m_regimes.count() - 1 && m_regimes[targetBlockEnd + 1].m_cycleId == targetCycleId) {
            targetBlockEnd++;
        }

        int destination = targetBlockEnd + 1;

        if (beginMoveRows(QModelIndex(), blockStart, blockEnd, QModelIndex(), destination)) {
            QList<Regime> movedItems;
            for (int i = 0; i < blockCount; ++i) {
                movedItems.append(m_regimes.takeAt(blockStart));
            }
            int insertPos = destination - blockCount;
            for (int i = 0; i < blockCount; ++i) {
                m_regimes.insert(insertPos + i, movedItems.at(i));
            }
            endMoveRows();

            for (int i = 0; i < blockCount; ++i) {
                newSelection.append(insertPos + i);
            }
        }
    }

    if (!newSelection.isEmpty()) {
        updateCycleIds();
        saveDataToJson();
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
    saveDataToJson();
}
