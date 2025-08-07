#pragma once

#include <QObject>
#include <QUrl>
#include "prototablemodel.h"

class RegimeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProtoTableModel* model READ model CONSTANT)
    Q_PROPERTY(QUrl currentFilePath READ currentFilePath WRITE setCurrentFilePath NOTIFY currentFilePathChanged)
    Q_PROPERTY(bool dirty READ dirty WRITE setDirty NOTIFY dirtyChanged)

public:
    explicit RegimeManager(QObject *parent = nullptr);

    ProtoTableModel* model();
    QUrl currentFilePath() const;
    void setCurrentFilePath(const QUrl &url);

    bool dirty() const;
    void setDirty(bool dirty);

    Q_INVOKABLE void loadDefaultRegimes();
    Q_INVOKABLE void importRegimes(const QUrl &filePath);
    Q_INVOKABLE void exportRegimes(const QUrl &filePath);
    Q_INVOKABLE void saveRegimes();
    Q_INVOKABLE void saveRegimesAs(const QUrl &filePath);

public slots:
    void onStateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds);

signals:
    void currentFilePathChanged();
    void dirtyChanged();
    void stateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds);

private:
    ProtoTableModel m_model;
    QUrl m_currentFilePath;
    bool m_dirty = false;
    QList<Regime> loadRegimesFromFile(const QString &filePath);
    void saveRegimesToFile(const QList<Regime> &regimes, const QString &filePath);
};
