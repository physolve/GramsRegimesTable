#include "regimemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QSet>

RegimeManager::RegimeManager(QObject *parent)
    : QObject{parent}, m_model(this)
{
    loadDefaultRegimes();
    connect(&m_model, &ProtoTableModel::dataChanged, this, [this]() { 
        setDirty(true); 
        // Force VisibleRegimeModel update when main model data changes
        refreshVisibleRegimes();
    });
    // Connect ProtoTableModel totalTimeChanged to VisibleRegimeModel update function
    connect(&m_model, &ProtoTableModel::totalTimeChanged, &m_visibleRegimeModel, &VisibleRegimeModel::notifyTimelineUpdate);
    // Forward VisibleRegimeModel signal to RegimeManager signal for backward compatibility
    connect(&m_visibleRegimeModel, &VisibleRegimeModel::timelineUpdateRequired, this, &RegimeManager::totalTimeChanged);

    // Removed hardcoded state change call - use setRegimeState() API instead
}

// delete late
void RegimeManager::testUpdatingRegimes(){
    // Start regime execution (repeat 0)
    startRegimeExecution(0);
    
    // Update condition progress for repeat 0
    // updateConditionProgress(0, 30, 0);  // 30 seconds into condition, repeat 0
    updateConditionProgress(0, 60, 0);  // Complete condition time, repeat 0
    
    // Confirm condition completion for repeat 0
    confirmConditionCompletion(0, 0);
    
    // Update regime execution progress for repeat 0
    // updateRegimeProgress(0, 600, 0);     // 45 seconds into execution, repeat 0
    updateRegimeProgress(0, 120, 0);    // Complete execution time, repeat 0
    
    // Complete first repeat (moves to repeat 1)
    completeCurrentRepeat(0, 0);
    
    // Now working on repeat 1 - update condition progress
    updateConditionProgress(0, 20, 1);  // 20 seconds into condition, repeat 1
    
    // Skip the second repeat
    skipCurrentRepeat(0, 1);
    
    qDebug() << "testUpdatingRegimes completed - regime should show repeat progression";
}

ProtoTableModel* RegimeManager::model()
{
    return &m_model;
}

QUrl RegimeManager::currentFilePath() const
{
    return m_currentFilePath;
}

void RegimeManager::setCurrentFilePath(const QUrl &url)
{
    if (m_currentFilePath != url) {
        m_currentFilePath = url;
        emit currentFilePathChanged();
    }
}

bool RegimeManager::dirty() const
{
    return m_dirty;
}

void RegimeManager::setDirty(bool dirty)
{
    if (m_dirty != dirty) {
        m_dirty = dirty;
        emit dirtyChanged();
    }
}

void RegimeManager::importRegimes(const QUrl &filePath)
{
    QList<Regime> regimes = loadRegimesFromFile(filePath.toLocalFile());
    m_model.clear();
    m_model.setRegimes(regimes);
    setCurrentFilePath(filePath);
    setDirty(false);
    emit totalTimeChanged();
}

void RegimeManager::exportRegimes(const QUrl &filePath)
{
    saveRegimesToFile(m_model.getRegimes(), filePath.toLocalFile());
}

void RegimeManager::saveRegimes()
{
    if (m_currentFilePath.isEmpty() || !m_currentFilePath.isValid()) return;
    saveRegimesToFile(m_model.getRegimes(), m_currentFilePath.toLocalFile());
    setDirty(false);
}

void RegimeManager::saveRegimesAs(const QUrl &filePath)
{
    setCurrentFilePath(filePath);
    saveRegimes();
}

void RegimeManager::loadDefaultRegimes()
{
    const QString defaultFilePath = "profile/regime_a.json";
    QList<Regime> regimes = loadRegimesFromFile(defaultFilePath);
    m_model.clear();
    m_model.setRegimes(regimes);
    setCurrentFilePath(QUrl::fromLocalFile(defaultFilePath));
    setDirty(false);
    emit totalTimeChanged();
}

QList<Regime> RegimeManager::loadRegimesFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file for reading:" << filePath;
        return {};
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonArray regimesArray = doc.array();

    QList<Regime> regimes;
    for (const QJsonValue &value : regimesArray) {
        regimes.append(Regime::fromJson(value.toObject()));
    }
    return regimes;
}

void RegimeManager::saveRegimesToFile(const QList<Regime> &regimes, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file for writing:" << filePath;
        return;
    }

    QJsonArray regimesArray;
    for (const auto &regime : regimes) {
        regimesArray.append(regime.toJson());
    }

    QJsonDocument doc(regimesArray);
    file.write(doc.toJson());
    file.close();
}

void RegimeManager::updateTotalTime()
{
    emit totalTimeChanged();
}

VisibleRegimeModel* RegimeManager::visibleRegimeModel()
{
    return &m_visibleRegimeModel;
}

void RegimeManager::updateVisibleRegimes(int visibleStartTime, int visibleEndTime)
{
    QList<Regime> visibleRegimes;
    int startTime = 0;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        Regime regime = m_model.getRegime(i);
        
        // Calculate condition time in seconds
        int conditionTimeInSeconds = 0;
        if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
            conditionTimeInSeconds = regime.m_condition.time * 60; // Convert minutes to seconds
        }
        
        // Total regime time includes condition time
        int totalRegimeTime = (conditionTimeInSeconds + regime.m_maxTime) * regime.m_repeatCount;
        int endTime = startTime + totalRegimeTime;
        
        if (endTime >= visibleStartTime && startTime <= visibleEndTime)
        {
            visibleRegimes.append(regime);
        }
        startTime = endTime;
    }
    m_visibleRegimeModel.setRegimes(visibleRegimes);
}

void RegimeManager::refreshVisibleRegimes()
{
    // Get current visible time range from TimeProgressBar or use full range
    int totalTime = getTotalEstimatedTime();
    if (totalTime > 0) {
        updateVisibleRegimes(0, totalTime);
    }
    
    // Emit signal for immediate UI refresh
    emit regimeDataUpdated();
}

void RegimeManager::onStateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds)
{
    if (regimeIndex < 0 || regimeIndex >= m_model.rowCount())
        return;

    m_model.setData(m_model.index(regimeIndex, 0), QVariant::fromValue(state), ProtoTableModel::StateRole);
    m_model.setData(m_model.index(regimeIndex, 0), timePassedInSeconds, ProtoTableModel::TimePassedInSecondsRole);
    emit totalTimeChanged();
}

void RegimeManager::setRegimeState(int regimeId, RegimeEnums::State state)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return;

    RegimeEnums::State currentState = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    if (currentState == state)
        return;

    m_model.setData(m_model.index(regimeId, 0), QVariant::fromValue(state), ProtoTableModel::StateRole);

    if (state == RegimeEnums::State::Done)
    {
        int repeatsDone = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
        m_model.setData(m_model.index(regimeId, 0), repeatsDone + 1, ProtoTableModel::RepeatsDoneRole);
    }
    else if (state == RegimeEnums::State::Skipped)
    {
        int repeatsSkipped = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsSkippedRole).toInt();
        m_model.setData(m_model.index(regimeId, 0), repeatsSkipped + 1, ProtoTableModel::RepeatsSkippedRole);
    }
    else if (state == RegimeEnums::State::Error)
    {
        int repeatsError = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsErrorRole).toInt();
        m_model.setData(m_model.index(regimeId, 0), repeatsError + 1, ProtoTableModel::RepeatsErrorRole);
    }
    
    // Emit the stateChanged signal for external modules to react
    emit stateChanged(regimeId, state, m_model.data(m_model.index(regimeId, 0), ProtoTableModel::TimePassedInSecondsRole).toInt());
    emit totalTimeChanged();
}

int RegimeManager::getRepeatsDone(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    return m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
}

int RegimeManager::getRepeatsSkipped(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    return m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsSkippedRole).toInt();
}

int RegimeManager::getRepeatsError(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    return m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsErrorRole).toInt();
}

int RegimeManager::getRepeatsLeft(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    int totalRepeats = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatRole).toInt();
    int repeatsDone = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
    return totalRepeats - repeatsDone;
}

int RegimeManager::getTimeLeftForRegime(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    int maxTime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::MaxTimeRole).toInt();
    int timePassed = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::TimePassedInSecondsRole).toInt();
    return maxTime - timePassed;
}

int RegimeManager::getTimeLeftForCycle(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    int cycleId = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CycleIdRole).toInt();
    if (cycleId == -1)
        return getTimeLeftForRegime(regimeId);

    int timeLeft = 0;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        if (m_model.data(m_model.index(i, 0), ProtoTableModel::CycleIdRole).toInt() == cycleId)
        {
            timeLeft += getTimeLeftForRegime(i) * getRepeatsLeft(i);
        }
    }
    return timeLeft;
}

int RegimeManager::getEstimatedTimeLeft() const
{
    int timeLeft = 0;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        timeLeft += getTimeLeftForRegime(i) * getRepeatsLeft(i);
    }
    return timeLeft;
}

int RegimeManager::getTotalTimeForRegime(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    
    // Calculate condition time in seconds (condition time is stored in minutes)
    int conditionTimeInSeconds = 0;
    if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
        conditionTimeInSeconds = regime.m_condition.time * 60; // Convert minutes to seconds
    }
    
    // Total time = (condition time + regime execution time) * repeat count
    return (conditionTimeInSeconds + regime.m_maxTime) * regime.m_repeatCount;
}

int RegimeManager::getElapsedTimeForRegime(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    return m_model.data(m_model.index(regimeId, 0), ProtoTableModel::TimePassedInSecondsRole).toInt();
}

int RegimeManager::getTotalTimeForCycle(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    Regime firstRegime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    if (firstRegime.m_cycleId == -1)
        return getTotalTimeForRegime(regimeId);

    int totalTimeForOneCycleIteration = 0;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        Regime currentRegime = m_model.data(m_model.index(i, 0), ProtoTableModel::RegimeRole).value<Regime>();
        if (currentRegime.m_cycleId == firstRegime.m_cycleId)
        {
            // Calculate condition time in seconds for each regime in the cycle
            int conditionTimeInSeconds = 0;
            if (currentRegime.m_condition.type == "time" || currentRegime.m_condition.type == "temp") {
                conditionTimeInSeconds = currentRegime.m_condition.time * 60; // Convert minutes to seconds
            }
            
            totalTimeForOneCycleIteration += (conditionTimeInSeconds + currentRegime.m_maxTime) * currentRegime.m_repeatCount;
        }
    }
    return totalTimeForOneCycleIteration * firstRegime.m_cycleRepeat;
}

int RegimeManager::getElapsedTimeForCycle(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    Regime firstRegime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    if (firstRegime.m_cycleId == -1)
        return getElapsedTimeForRegime(regimeId);

    int elapsedTime = 0;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        Regime currentRegime = m_model.data(m_model.index(i, 0), ProtoTableModel::RegimeRole).value<Regime>();
        if (currentRegime.m_cycleId == firstRegime.m_cycleId)
        {
            elapsedTime += getElapsedTimeForRegime(i);
        }
    }
    return elapsedTime;
}

// ========== EXTERNAL MODULE API IMPLEMENTATION ==========

bool RegimeManager::startRegimeExecution(int regimeId)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "startRegimeExecution: Invalid regime ID" << regimeId;
        return false;
    }
    
    RegimeEnums::State currentState = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    if (currentState == RegimeEnums::State::Running) {
        qWarning() << "startRegimeExecution: Regime" << regimeId << "is already running";
        return false;
    }
    
    // Reset execution tracking
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::CurrentRepeatRole);
    m_model.setData(m_model.index(regimeId, 0), false, ProtoTableModel::ConditionCompletedRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::ConditionTimePassedRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RegimeTimePassedRole);
    
    // Set state to running
    setRegimeState(regimeId, RegimeEnums::State::Running);
    
    // Force immediate UI update
    refreshVisibleRegimes();
    
    qDebug() << "Started execution for regime" << regimeId;
    return true;
}

bool RegimeManager::updateConditionProgress(int regimeId, int conditionTimeElapsed, int currentRepeat)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "updateConditionProgress: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Validate current repeat
    int actualCurrentRepeat = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    if (actualCurrentRepeat != currentRepeat) {
        qWarning() << "updateConditionProgress: Repeat mismatch. Expected" << actualCurrentRepeat << "got" << currentRepeat;
        return false;
    }
    
    // Validate state
    RegimeEnums::State state = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    if (state != RegimeEnums::State::Running) {
        qWarning() << "updateConditionProgress: Regime" << regimeId << "is not running";
        return false;
    }
    
    // Validate condition not completed
    bool conditionCompleted = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::ConditionCompletedRole).toBool();
    if (conditionCompleted) {
        qWarning() << "updateConditionProgress: Condition already completed for regime" << regimeId;
        return false;
    }
    
    // Get condition time limit
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    int conditionTimeLimit = 0;
    if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
        conditionTimeLimit = regime.m_condition.time * 60; // Convert to seconds
    }
    
    // Validate elapsed time
    if (conditionTimeElapsed < 0 || conditionTimeElapsed > conditionTimeLimit) {
        qWarning() << "updateConditionProgress: Invalid elapsed time" << conditionTimeElapsed << "(limit:" << conditionTimeLimit << ")";
        return false;
    }
    
    // Update condition progress
    m_model.setData(m_model.index(regimeId, 0), conditionTimeElapsed, ProtoTableModel::ConditionTimePassedRole);
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

bool RegimeManager::confirmConditionCompletion(int regimeId, int currentRepeat)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "confirmConditionCompletion: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Validate current repeat
    int actualCurrentRepeat = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    if (actualCurrentRepeat != currentRepeat) {
        qWarning() << "confirmConditionCompletion: Repeat mismatch. Expected" << actualCurrentRepeat << "got" << currentRepeat;
        return false;
    }
    
    // Validate state
    RegimeEnums::State state = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    if (state != RegimeEnums::State::Running) {
        qWarning() << "confirmConditionCompletion: Regime" << regimeId << "is not running";
        return false;
    }
    
    // Mark condition as completed
    m_model.setData(m_model.index(regimeId, 0), true, ProtoTableModel::ConditionCompletedRole);
    
    // Set condition time to full duration
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
        int conditionTimeLimit = regime.m_condition.time * 60; // Convert to seconds
        m_model.setData(m_model.index(regimeId, 0), conditionTimeLimit, ProtoTableModel::ConditionTimePassedRole);
    }
    
    qDebug() << "Condition completed for regime" << regimeId << "repeat" << currentRepeat;
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

bool RegimeManager::updateRegimeProgress(int regimeId, int regimeTimeElapsed, int currentRepeat)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "updateRegimeProgress: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Validate current repeat
    int actualCurrentRepeat = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    if (actualCurrentRepeat != currentRepeat) {
        qWarning() << "updateRegimeProgress: Repeat mismatch. Expected" << actualCurrentRepeat << "got" << currentRepeat;
        return false;
    }
    
    // Validate state
    RegimeEnums::State state = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    if (state != RegimeEnums::State::Running) {
        qWarning() << "updateRegimeProgress: Regime" << regimeId << "is not running";
        return false;
    }
    
    // Get regime execution time limit
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    int regimeTimeLimit = regime.m_maxTime;
    
    // Validate elapsed time
    if (regimeTimeElapsed < 0 || regimeTimeElapsed > regimeTimeLimit) {
        qWarning() << "updateRegimeProgress: Invalid elapsed time" << regimeTimeElapsed << "(limit:" << regimeTimeLimit << ")";
        return false;
    }
    
    // Update regime progress
    m_model.setData(m_model.index(regimeId, 0), regimeTimeElapsed, ProtoTableModel::RegimeTimePassedRole);
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

bool RegimeManager::completeCurrentRepeat(int regimeId, int currentRepeat)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "completeCurrentRepeat: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Validate current repeat
    int actualCurrentRepeat = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    if (actualCurrentRepeat != currentRepeat) {
        qWarning() << "completeCurrentRepeat: Repeat mismatch. Expected" << actualCurrentRepeat << "got" << currentRepeat;
        return false;
    }
    
    // Validate state
    RegimeEnums::State state = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    if (state != RegimeEnums::State::Running) {
        qWarning() << "completeCurrentRepeat: Regime" << regimeId << "is not running";
        return false;
    }
    
    // Increment repeats done
    int repeatsDone = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
    m_model.setData(m_model.index(regimeId, 0), repeatsDone + 1, ProtoTableModel::RepeatsDoneRole);
    
    // Get total repeats needed
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    int totalRepeats = regime.m_repeatCount;
    
    if (repeatsDone + 1 >= totalRepeats) {
        // All repeats completed - mark regime as done
        setRegimeState(regimeId, RegimeEnums::State::Done);
        qDebug() << "Regime" << regimeId << "completed all repeats";
    } else {
        // Move to next repeat
        m_model.setData(m_model.index(regimeId, 0), currentRepeat + 1, ProtoTableModel::CurrentRepeatRole);
        m_model.setData(m_model.index(regimeId, 0), false, ProtoTableModel::ConditionCompletedRole);
        m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::ConditionTimePassedRole);
        m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RegimeTimePassedRole);
        qDebug() << "Regime" << regimeId << "moved to repeat" << (currentRepeat + 1);
    }
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

bool RegimeManager::completeRegimeExecution(int regimeId)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "completeRegimeExecution: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Set state to done
    setRegimeState(regimeId, RegimeEnums::State::Done);
    
    // Update repeats done to match total repeats
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    m_model.setData(m_model.index(regimeId, 0), regime.m_repeatCount, ProtoTableModel::RepeatsDoneRole);
    
    qDebug() << "Regime" << regimeId << "execution completed";
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

bool RegimeManager::skipCurrentRepeat(int regimeId, int currentRepeat)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "skipCurrentRepeat: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Validate current repeat
    int actualCurrentRepeat = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    if (actualCurrentRepeat != currentRepeat) {
        qWarning() << "skipCurrentRepeat: Repeat mismatch. Expected" << actualCurrentRepeat << "got" << currentRepeat;
        return false;
    }
    
    // Increment repeats skipped
    int repeatsSkipped = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsSkippedRole).toInt();
    m_model.setData(m_model.index(regimeId, 0), repeatsSkipped + 1, ProtoTableModel::RepeatsSkippedRole);
    
    // Get total repeats needed
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    int totalRepeats = regime.m_repeatCount;
    int repeatsDone = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
    
    if (repeatsDone + repeatsSkipped + 1 >= totalRepeats) {
        // All repeats processed - mark regime as done
        setRegimeState(regimeId, RegimeEnums::State::Done);
        qDebug() << "Regime" << regimeId << "completed (with skips)";
    } else {
        // Move to next repeat
        m_model.setData(m_model.index(regimeId, 0), currentRepeat + 1, ProtoTableModel::CurrentRepeatRole);
        m_model.setData(m_model.index(regimeId, 0), false, ProtoTableModel::ConditionCompletedRole);
        m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::ConditionTimePassedRole);
        m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RegimeTimePassedRole);
        qDebug() << "Regime" << regimeId << "skipped repeat" << currentRepeat << "moved to repeat" << (currentRepeat + 1);
    }
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

bool RegimeManager::markRepeatAsError(int regimeId, int currentRepeat)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "markRepeatAsError: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Validate current repeat
    int actualCurrentRepeat = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    if (actualCurrentRepeat != currentRepeat) {
        qWarning() << "markRepeatAsError: Repeat mismatch. Expected" << actualCurrentRepeat << "got" << currentRepeat;
        return false;
    }
    
    // Increment repeats error
    int repeatsError = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsErrorRole).toInt();
    m_model.setData(m_model.index(regimeId, 0), repeatsError + 1, ProtoTableModel::RepeatsErrorRole);
    
    // Get total repeats needed
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    int totalRepeats = regime.m_repeatCount;
    int repeatsDone = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
    int repeatsSkipped = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsSkippedRole).toInt();
    
    if (repeatsDone + repeatsSkipped + repeatsError + 1 >= totalRepeats) {
        // All repeats processed - mark regime as error
        setRegimeState(regimeId, RegimeEnums::State::Error);
        qDebug() << "Regime" << regimeId << "completed with errors";
    } else {
        // Move to next repeat
        m_model.setData(m_model.index(regimeId, 0), currentRepeat + 1, ProtoTableModel::CurrentRepeatRole);
        m_model.setData(m_model.index(regimeId, 0), false, ProtoTableModel::ConditionCompletedRole);
        m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::ConditionTimePassedRole);
        m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RegimeTimePassedRole);
        qDebug() << "Regime" << regimeId << "error in repeat" << currentRepeat << "moved to repeat" << (currentRepeat + 1);
    }
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

QVariantMap RegimeManager::getRegimeExecutionInfo(int regimeId) const
{
    QVariantMap info;
    
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "getRegimeExecutionInfo: Invalid regime ID" << regimeId;
        return info;
    }
    
    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    
    info["regimeId"] = regimeId;
    info["name"] = regime.m_name;
    info["state"] = static_cast<int>(regime.m_state);
    info["currentRepeat"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::CurrentRepeatRole).toInt();
    info["totalRepeats"] = regime.m_repeatCount;
    info["conditionCompleted"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::ConditionCompletedRole).toBool();
    info["conditionTimePassed"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::ConditionTimePassedRole).toInt();
    info["regimeTimePassed"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeTimePassedRole).toInt();
    info["repeatsDone"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsDoneRole).toInt();
    info["repeatsSkipped"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsSkippedRole).toInt();
    info["repeatsError"] = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RepeatsErrorRole).toInt();
    
    // Condition info
    info["conditionType"] = regime.m_condition.type;
    info["conditionTime"] = regime.m_condition.time;
    info["conditionTemp"] = regime.m_condition.temp;
    
    // Time limits
    int conditionTimeLimit = 0;
    if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
        conditionTimeLimit = regime.m_condition.time * 60;
    }
    info["conditionTimeLimit"] = conditionTimeLimit;
    info["regimeTimeLimit"] = regime.m_maxTime;
    
    return info;
}

bool RegimeManager::resetRegimeExecution(int regimeId)
{
    if (regimeId < 0 || regimeId >= m_model.rowCount()) {
        qWarning() << "resetRegimeExecution: Invalid regime ID" << regimeId;
        return false;
    }
    
    // Reset all execution tracking
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::CurrentRepeatRole);
    m_model.setData(m_model.index(regimeId, 0), false, ProtoTableModel::ConditionCompletedRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::ConditionTimePassedRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RegimeTimePassedRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RepeatsDoneRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RepeatsSkippedRole);
    m_model.setData(m_model.index(regimeId, 0), 0, ProtoTableModel::RepeatsErrorRole);
    
    // Set state to waiting
    setRegimeState(regimeId, RegimeEnums::State::Waiting);
    
    qDebug() << "Reset execution for regime" << regimeId;
    
    // Force immediate UI update
    refreshVisibleRegimes();
    emit totalTimeChanged();
    return true;
}

int RegimeManager::getConditionTimePassedForRegime(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    
    // Get condition time in seconds
    int conditionTimeInSeconds = 0;
    if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
        conditionTimeInSeconds = regime.m_condition.time * 60; // Convert minutes to seconds
    }
    
    if (conditionTimeInSeconds == 0) {
        return 0; // No condition time
    }
    
    int totalTimePassed = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::TimePassedInSecondsRole).toInt();
    
    // Condition time is the first part of the total time
    return qMin(totalTimePassed, conditionTimeInSeconds);
}

int RegimeManager::getConditionTimeLeftForRegime(int regimeId) const
{
    if (regimeId < 0 || regimeId >= m_model.rowCount())
        return 0;

    Regime regime = m_model.data(m_model.index(regimeId, 0), ProtoTableModel::RegimeRole).value<Regime>();
    
    // Get condition time in seconds
    int conditionTimeInSeconds = 0;
    if (regime.m_condition.type == "time" || regime.m_condition.type == "temp") {
        conditionTimeInSeconds = regime.m_condition.time * 60; // Convert minutes to seconds
    }
    
    if (conditionTimeInSeconds == 0) {
        return 0; // No condition time
    }
    
    int conditionTimePassed = getConditionTimePassedForRegime(regimeId);
    return conditionTimeInSeconds - conditionTimePassed;
}

int RegimeManager::getTotalEstimatedTime() const
{
    int totalTime = 0;
    QSet<int> processedCycleIds;

    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        Regime regime = m_model.data(m_model.index(i, 0), ProtoTableModel::RegimeRole).value<Regime>(); // to const auto& ?
        if (regime.m_cycleId != -1) {
            if (!processedCycleIds.contains(regime.m_cycleId)) {
                totalTime += getTotalTimeForCycle(i);
                processedCycleIds.insert(regime.m_cycleId);
            }
        } else {
            totalTime += getTotalTimeForRegime(i);
        }
    }
    return totalTime;
}

int RegimeManager::getTotalElapsedTime() const
{
    int elapsedTime = 0;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        elapsedTime += getElapsedTimeForRegime(i);
    }
    return elapsedTime;
}