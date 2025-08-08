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
        // Maximum time for the regime in minutes
        MaxTimeRole,
        CycleRowCountRole,
        CycleStatusRole,
        CycleRepeatRole,
        StateRole,
        // Time passed in seconds
        TimePassedInSecondsRole,
        RepeatsDoneRole,
        RepeatsSkippedRole,
        RepeatsErrorRole,
        CycleIdRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    QHash<int, QByteArray> roleNames() const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_INVOKABLE void setRegimes(const QList<Regime> &regimes);
    Q_INVOKABLE QList<Regime> getRegimes() const;

    Q_INVOKABLE void groupRows(QVariantList rows);
    Q_INVOKABLE void ungroupRows(QVariantList rows);
    Q_INVOKABLE QVariantList moveSelection(QVariantList rows, bool up);

    Q_INVOKABLE void addRow(const QString &regimeName);
    Q_INVOKABLE void deleteRows(QVariantList rows);
    Q_INVOKABLE void clear();
    Q_INVOKABLE bool isSelectionGroupable(QVariantList rows) const;
    Q_INVOKABLE bool isSelectionUngroupable(QVariantList rows) const;
    Q_INVOKABLE bool isMoveUpEnabled(QVariantList rows) const;
    Q_INVOKABLE bool isMoveDownEnabled(QVariantList rows) const;

    Q_INVOKABLE Regime getRegime(int row) const;
    Q_INVOKABLE QVariant get(int row, const QByteArray& roleName) const;
    Q_INVOKABLE bool isAnyRegimeRunning() const;

public slots:
    Q_INVOKABLE int getRowCount() { return m_regimes.count(); }

signals:
    void selectionShouldBeCleared();

private:
    void updateCycleIds();
    void checkAndUpdateRunningState();
    int getBlockStart(QVariantList rows) const;
    int getBlockEnd(QVariantList rows) const;

    QList<Regime> m_regimes;
    QStringList m_columnNames;
    bool m_isAnyRegimeRunning = false;
};
