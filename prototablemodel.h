#pragma once

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QMap>
#include "regime.h"

class ProtoTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ProtoTableModel(QObject *parent = nullptr);

    enum Role {
        RegimeRole = Qt::UserRole + 1,
        ConditionRole,
        RepeatRole,
        MaxTimeRole,
        SpanRole,
        CycleStatusRole,
        CycleRepeatRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_INVOKABLE void saveDataToJson();
    Q_INVOKABLE void groupRows(QVariantList rows);
    Q_INVOKABLE void ungroupRows(QVariantList rows);

signals:
    void regimeButtonClicked(QVariant regime);
    Q_INVOKABLE void updateRow(int row);

private:
    void loadDataFromJson();

    QList<Regime> m_regimes;
    QStringList m_columnNames;
};
