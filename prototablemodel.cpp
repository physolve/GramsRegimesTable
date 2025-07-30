#include "prototablemodel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

ProtoTableModel::ProtoTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnNames << "Режим" << "Условие" << "Повтор" << "Макс. время";
    loadDataFromJson();
}

ProtoTableModel::~ProtoTableModel()
{
    clearRegimes();
}

void ProtoTableModel::clearRegimes()
{
    beginResetModel();
    qDeleteAll(m_regimes);
    m_regimes.clear();
    m_data.clear();
    endResetModel();
}

void ProtoTableModel::loadDataFromJson()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/regime_a.json";
    QFile file(filePath);

    if (!file.exists()) {
        // Fallback to resource file if it doesn't exist in the writable location
        if (!QDir().exists(":/prototype_table/")) {
            qWarning("Resource path :/prototype_table/ does not exist.");
            return;
        }
        file.setFileName(":/prototype_table/regime_a.json");
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open regime_a.json from" << file.fileName();
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject json = doc.object();
    QJsonArray regimesArray = json["regimes"].toArray();

    clearRegimes();
    beginInsertRows(QModelIndex(), 0, regimesArray.count() - 1);

    for (const QJsonValue &value : regimesArray) {
        QJsonObject obj = value.toObject();
        Regime *regime = new Regime();
        regime->name = obj["name"].toString();

        QJsonObject conditionObj = obj["condition"].toObject();
        regime->condition.type = conditionObj["type"].toString();
        regime->condition.temp = conditionObj["temp"].toDouble();
        regime->condition.time = conditionObj["time"].toDouble();

        QJsonObject repeatObj = obj["repeat"].toObject();
        regime->repeat.count = repeatObj["count"].toInt();

        m_regimes.append(regime);

        QStringList rowData;
        rowData << regime->name
                << regime->condition.type
                << QString::number(regime->repeat.count)
                << QString::number(regime->condition.time);
        m_data.append(rowData);
    }

    endInsertRows();
}

void ProtoTableModel::saveDataToJson()
{
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = dir.filePath("regime_a.json");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file for writing:" << filePath;
        return;
    }

    QJsonArray regimesArray;
    for (const Regime* regime : m_regimes) {
        QJsonObject regimeObj;
        regimeObj["name"] = regime->name;

        QJsonObject conditionObj;
        conditionObj["type"] = regime->condition.type;
        conditionObj["temp"] = regime->condition.temp;
        conditionObj["time"] = regime->condition.time;
        regimeObj["condition"] = conditionObj;

        QJsonObject repeatObj;
        repeatObj["count"] = regime->repeat.count;
        regimeObj["repeat"] = repeatObj;

        regimesArray.append(regimeObj);
    }

    QJsonObject root;
    root["regimes"] = regimesArray;
    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
    qInfo() << "Saved data to" << filePath;
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

    const Regime *regime = m_regimes.at(index.row());

    if (role == Qt::DisplayRole) {
        return m_data.at(index.row()).at(index.column());
    }

    if (role == ConditionRole) {
        QVariantMap conditionMap;
        conditionMap["type"] = regime->condition.type;
        conditionMap["temp"] = regime->condition.temp;
        conditionMap["time"] = regime->condition.time;
        return conditionMap;
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
        { ConditionRole, "condition" }
    };
}

Qt::ItemFlags ProtoTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void ProtoTableModel::updateCondition(int row, const QVariant &newCondition)
{
    if (row < 0 || row >= m_regimes.count()) {
        return;
    }

    Regime *regime = m_regimes.at(row);
    QVariantMap conditionMap = newCondition.toMap();

    regime->condition.type = conditionMap["type"].toString();
    regime->condition.temp = conditionMap["temp"].toDouble();
    regime->condition.time = conditionMap["time"].toDouble();

    // Update the display data as well
    m_data[row][1] = regime->condition.type;
    m_data[row][3] = QString::number(regime->condition.time);

    emit dataChanged(index(row, 0), index(row, columnCount() - 1), {Qt::DisplayRole, ConditionRole});
}