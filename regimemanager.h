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
    
    /// Forces refresh of VisibleRegimeModel with current data
    void refreshVisibleRegimes();
    
    /// Returns the condition time passed for a specific regime in seconds
    Q_INVOKABLE int getConditionTimePassedForRegime(int regimeId) const;
    /// Returns the condition time left for a specific regime in seconds  
    Q_INVOKABLE int getConditionTimeLeftForRegime(int regimeId) const;
    
    // ========== EXTERNAL MODULE API ==========
    
    /**
     * @brief Starts execution of a regime (sets state to Running)
     * @param regimeId The regime to start
     * @return true if successfully started, false if invalid regime or already running
     */
    Q_INVOKABLE bool startRegimeExecution(int regimeId);
    
    /**
     * @brief Updates condition progress for a running regime
     * @param regimeId The regime ID
     * @param conditionTimeElapsed Time elapsed in condition phase (seconds)
     * @param currentRepeat Current repeat number (0-based) for validation
     * @return true if update successful, false if validation failed
     */
    Q_INVOKABLE bool updateConditionProgress(int regimeId, int conditionTimeElapsed, int currentRepeat);
    
    /**
     * @brief Confirms condition completion and starts regime execution phase
     * @param regimeId The regime ID
     * @param currentRepeat Current repeat number (0-based) for validation
     * @return true if transition successful, false if validation failed
     */
    Q_INVOKABLE bool confirmConditionCompletion(int regimeId, int currentRepeat);
    
    /**
     * @brief Updates regime execution progress
     * @param regimeId The regime ID
     * @param regimeTimeElapsed Time elapsed in execution phase (seconds)
     * @param currentRepeat Current repeat number (0-based) for validation
     * @return true if update successful, false if validation failed
     */
    Q_INVOKABLE bool updateRegimeProgress(int regimeId, int regimeTimeElapsed, int currentRepeat);
    
    /**
     * @brief Completes current repeat and moves to next repeat or regime
     * @param regimeId The regime ID
     * @param currentRepeat Current repeat number (0-based) for validation
     * @return true if completion successful, false if validation failed
     */
    Q_INVOKABLE bool completeCurrentRepeat(int regimeId, int currentRepeat);
    
    /**
     * @brief Completes entire regime execution (all repeats done)
     * @param regimeId The regime ID
     * @return true if completion successful, false if invalid regime
     */
    Q_INVOKABLE bool completeRegimeExecution(int regimeId);
    
    /**
     * @brief Skips current repeat and moves to next
     * @param regimeId The regime ID
     * @param currentRepeat Current repeat number (0-based) for validation
     * @return true if skip successful, false if validation failed
     */
    Q_INVOKABLE bool skipCurrentRepeat(int regimeId, int currentRepeat);
    
    /**
     * @brief Marks current repeat as error and moves to next
     * @param regimeId The regime ID
     * @param currentRepeat Current repeat number (0-based) for validation
     * @return true if error marking successful, false if validation failed
     */
    Q_INVOKABLE bool markRepeatAsError(int regimeId, int currentRepeat);
    
    /**
     * @brief Gets current execution info for a regime
     * @param regimeId The regime ID
     * @return QVariantMap with execution details (currentRepeat, conditionCompleted, etc.)
     */
    Q_INVOKABLE QVariantMap getRegimeExecutionInfo(int regimeId) const;
    
    /**
     * @brief Resets regime to initial state (for restart)
     * @param regimeId The regime ID
     * @return true if reset successful, false if invalid regime
     */
    Q_INVOKABLE bool resetRegimeExecution(int regimeId);
    
    // delete late
    Q_INVOKABLE void testUpdatingRegimes();

public slots:
    void onStateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds);

signals:
    void currentFilePathChanged();
    void dirtyChanged();
    void totalTimeChanged();
    void stateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds);
    void regimeDataUpdated(); // Signal for immediate UI refresh

private:
    ProtoTableModel m_model;
    VisibleRegimeModel m_visibleRegimeModel;
    QUrl m_currentFilePath;
    bool m_dirty = false;
    QList<Regime> loadRegimesFromFile(const QString &filePath);
    void saveRegimesToFile(const QList<Regime> &regimes, const QString &filePath);
};
