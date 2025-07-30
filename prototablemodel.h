#pragma once

#include <QAbstractTableModel>
#include <QtQmlIntegration>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>

class ProtoTableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ProtoTableModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    

    enum Roles {
        ConditionRole = Qt::UserRole + 1
    };

    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_INVOKABLE void updateCondition(int row, const QVariant &newCondition);
    Q_INVOKABLE void saveDataToJson();

private:
    void loadDataFromJson();
    QStringList m_columnNames;
    QList<QStringList> m_data;
    QJsonArray m_regimes;
};

