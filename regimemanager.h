#pragma once

#include <QObject>
#include <QUrl>
#include "prototablemodel.h"

class RegimeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProtoTableModel* model READ model CONSTANT)
    Q_PROPERTY(QUrl currentFilePath READ currentFilePath WRITE setCurrentFilePath NOTIFY currentFilePathChanged)

public:
    explicit RegimeManager(QObject *parent = nullptr);

    ProtoTableModel* model();
    QUrl currentFilePath() const;
    void setCurrentFilePath(const QUrl &url);

    Q_INVOKABLE void loadDefaultRegimes();
    Q_INVOKABLE void importRegimes(const QUrl &filePath);
    Q_INVOKABLE void exportRegimes(const QUrl &filePath);
    Q_INVOKABLE void saveRegimes();
    Q_INVOKABLE void saveRegimesAs(const QUrl &filePath);

signals:
    void currentFilePathChanged();

private:
    ProtoTableModel m_model;
    QUrl m_currentFilePath;
    QList<Regime> loadRegimesFromFile(const QString &filePath);
    void saveRegimesToFile(const QList<Regime> &regimes, const QString &filePath);
};
