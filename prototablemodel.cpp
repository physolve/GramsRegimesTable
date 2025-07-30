#include "prototablemodel.h"

void ProtoTableModel::loadDataFromJson()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/regime_a.json";
    QFile file(filePath);
    if (!file.exists()) {
        file.setFileName(":/prototype_table/regime_a.json");
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open regime_a.json");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject json = doc.object();
    m_regimes = json["regimes"].toArray();

    beginResetModel();
    m_columnNames.clear();
    m_data.clear();

    m_columnNames << "Режим" << "Условие" << "Повтор" << "Макс. время";

    for (const QJsonValue &value : m_regimes) {
        QJsonObject obj = value.toObject();
        QStringList row;
        row.append(obj["name"].toString());
        row.append(obj["condition"].toObject()["type"].toString());
        row.append(QString::number(obj["repeat"].toObject()["count"].toInt()));
        row.append(QString::number(obj["condition"].toObject()["time"].toInt()));
        m_data.append(row);
    }

    endResetModel();
}

void ProtoTableModel::saveDataToJson()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/regime_a.json";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file for writing");
        return;
    }

    QJsonObject root;
    root["regimes"] = m_regimes;
    QJsonDocument doc(root);
    file.write(doc.toJson());
}

ProtoTableModel::ProtoTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    loadDataFromJson();
}

int ProtoTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.count();
}

int ProtoTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_columnNames.count();
}

QVariant ProtoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return m_data.at(index.row()).at(index.column());
    }

    if (role == ConditionRole) {
        QJsonObject regime = m_regimes[index.row()].toObject();
        return regime["condition"].toObject();
    }
    return QVariant();
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
        { Qt::EditRole, "edit" },
        { ConditionRole, "condition" }
    };
}

Qt::ItemFlags ProtoTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void ProtoTableModel::updateCondition(int row, const QVariant &newCondition)
{
    if (row < 0 || row >= m_regimes.count()) {
        return;
    }

    QJsonObject newConditionObj = newCondition.toJsonObject();
    m_regimes[row].toObject()["condition"] = newConditionObj;

    // Update the display data as well
    m_data[row][1] = newConditionObj["type"].toString();
    m_data[row][3] = QString::number(newConditionObj["time"].toInt());

    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}
