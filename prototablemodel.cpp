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

    int newCycleId = 0;
    for (const auto &regime : m_regimes) {
        if (regime.m_cycleId > newCycleId) {
            newCycleId = regime.m_cycleId;
        }
    }
    newCycleId++;

    for (const QVariant &rowVariant : rows) {
        int row = rowVariant.toInt();
        if (row >= 0 && row < m_regimes.count()) {
            m_regimes[row].m_cycleId = newCycleId;
        }
    }

    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
}

void ProtoTableModel::ungroupRows(QVariantList rows)
{
    if (rows.isEmpty()) return;

    for (const QVariant &rowVariant : rows) {
        int row = rowVariant.toInt();
        if (row >= 0 && row < m_regimes.count()) {
            m_regimes[row].m_cycleId = -1;
        }
    }

    emit dataChanged(index(0, 0), index(m_regimes.count() - 1, columnCount() - 1));
}
