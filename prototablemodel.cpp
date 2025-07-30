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
    QJsonObject json = doc.object();
    QJsonArray regimesArray = json["regimes"].toArray();

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

    QJsonObject root;
    root["regimes"] = regimesArray;
    QJsonDocument doc(root);
    file.write(doc.toJson());

    file.close();
}

ProtoTableModel::ProtoTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnNames << "Режим" << "Условие" << "Повтор" << "Макс. время";
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
        case 2: return regime.m_repeatCount;
        case 3: return regime.m_condition.time;
        default: return QVariant();
        }
    }

    if (role == RepeatRole) {
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

    return QVariant();
}

bool ProtoTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_regimes.count()) {
        return false;
    }
    
    if (role == RepeatRole) {
        m_regimes[index.row()].m_repeatCount = value.toInt();
        emit dataChanged(index, index, {role, Qt::DisplayRole});
        return true;
    }

    if (role == MaxTimeRole) {
        m_regimes[index.row()].m_maxTime = value.toInt();
        emit dataChanged(index, index, {role, Qt::DisplayRole});
        return true;
    }

    if (role == ConditionRole) {
        m_regimes[index.row()].m_condition = value.value<Condition>();
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
        { MaxTimeRole, "max_time" }
    };
}

Qt::ItemFlags ProtoTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}