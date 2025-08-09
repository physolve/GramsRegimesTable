#pragma once

#include <QObject>
#include <QUrl>
#include "prototablemodel.h"
#include "visibleregimemodel.h"

class RegimeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProtoTableModel* model READ model CONSTANT)
    Q_PROPERTY(QUrl currentFilePath READ currentFilePath WRITE setCurrentFilePath NOTIFY currentFilePathChanged)
    Q_PROPERTY(bool dirty READ dirty WRITE setDirty NOTIFY dirtyChanged)
    Q_PROPERTY(VisibleRegimeModel* visibleRegimeModel READ visibleRegimeModel CONSTANT)

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
    void saveRegimesAs(const QUrl &filePath);

    VisibleRegimeModel* visibleRegimeModel();

    Q_INVOKABLE void setRegimeState(int regimeId, RegimeEnums::State state);
    Q_INVOKABLE int getRepeatsDone(int regimeId) const;
    Q_INVOKABLE int getRepeatsSkipped(int regimeId) const;
    Q_INVOKABLE int getRepeatsError(int regimeId) const;
    Q_INVOKABLE int getRepeatsLeft(int regimeId) const;
    // Returns the time left for a specific regime in seconds.
    Q_INVOKABLE int getTimeLeftForRegime(int regimeId) const;
    // Returns the time left for a specific cycle in seconds.
    Q_INVOKABLE int getTimeLeftForCycle(int regimeId) const;
    // Returns the total estimated time left for all regimes in seconds.
    Q_INVOKABLE int getEstimatedTimeLeft() const;

    // Returns the total time for a specific regime in seconds.
    Q_INVOKABLE int getTotalTimeForRegime(int regimeId) const;
    // Returns the elapsed time for a specific regime in seconds.
    Q_INVOKABLE int getElapsedTimeForRegime(int regimeId) const;
    // Returns the total time for a specific cycle in seconds.
    Q_INVOKABLE int getTotalTimeForCycle(int regimeId) const;
    // Returns the elapsed time for a specific cycle in seconds.
    Q_INVOKABLE int getElapsedTimeForCycle(int regimeId) const;
    // Returns the total estimated time for all regimes in seconds.
    Q_INVOKABLE int getTotalEstimatedTime() const;
    // Returns the total elapsed time for all regimes in seconds.
    Q_INVOKABLE int getTotalElapsedTime() const;

    Q_INVOKABLE void updateTotalTime();
    Q_INVOKABLE void updateVisibleRegimes(int visibleStartTime, int visibleEndTime);

public slots:
    void onStateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds);

signals:
    void currentFilePathChanged();
    void dirtyChanged();
    void stateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds);
    void totalTimeChanged();

private:
    ProtoTableModel m_model;
    VisibleRegimeModel m_visibleRegimeModel;
    QUrl m_currentFilePath;
    bool m_dirty = false;
    QList<Regime> loadRegimesFromFile(const QString &filePath);
    void saveRegimesToFile(const QList<Regime> &regimes, const QString &filePath);
};
