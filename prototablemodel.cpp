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
    saveDataToJson();
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
        { SpanRole, "span" },
        { CycleStatusRole, "cycle_status" },
        { CycleRepeatRole, "cycle_repeat" }
    };
}

Qt::ItemFlags ProtoTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
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
        } else {
            qWarning() << "groupRows: Invalid row index" << row;
        }
    }

    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1), {CycleStatusRole, SpanRole});
    emit selectionShouldBeCleared();
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
        }
    }

    updateCycleIds();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1), {SpanRole, RepeatRole, CycleRepeatRole, CycleStatusRole});
    emit selectionShouldBeCleared();
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
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
    saveDataToJson();
}

void ProtoTableModel::deleteRows(QVariantList rows)
{
    if (rows.isEmpty()) return;

    QList<int> rowIndices;
    for (const QVariant &row : rows) {
        rowIndices.append(row.toInt());
    }
    std::sort(rowIndices.begin(), rowIndices.end());

    QSet<int> cyclesToDelete;
    for (int rowIndex : rowIndices) {
        if (m_regimes[rowIndex].m_cycleId != -1) {
            cyclesToDelete.insert(m_regimes[rowIndex].m_cycleId);
        }
    }

    beginResetModel();

    for (int i = m_regimes.count() - 1; i >= 0; --i) {
        if (rowIndices.contains(i) || cyclesToDelete.contains(m_regimes[i].m_cycleId)) {
            m_regimes.removeAt(i);
        }
    }

    updateCycleIds();
    endResetModel();
    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
}

void ProtoTableModel::clear()
{
    beginResetModel();
    m_regimes.clear();
    endResetModel();
}

bool ProtoTableModel::isSelectionGroupable(QVariantList rows) const
{
    if (rows.count() < 2) return false;

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
    return blockStart > 0;
}

bool ProtoTableModel::isMoveDownEnabled(QVariantList rows) const
{
    if (rows.isEmpty()) return false;
    int blockEnd = getBlockEnd(rows);
    return blockEnd != -1 && blockEnd < m_regimes.count() - 1;
}

QVariant ProtoTableModel::get(int row, const QByteArray& roleName) const
{
    int role = roleNames().key(roleName);
    if (role == 0) { // roleName not found
        return QVariant();
    }
    return data(index(row, 0), role);
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