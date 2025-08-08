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
    connect(&m_model, &ProtoTableModel::dataChanged, this, [this]() { setDirty(true); });
    onStateChanged(0, RegimeEnums::State::Running, 540);
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

void RegimeManager::onStateChanged(int regimeIndex, RegimeEnums::State state, int timePassedInSeconds)
{
    if (regimeIndex < 0 || regimeIndex >= m_model.rowCount())
        return;

    m_model.setData(m_model.index(regimeIndex, 0), QVariant::fromValue(state), ProtoTableModel::StateRole);
    m_model.setData(m_model.index(regimeIndex, 0), timePassedInSeconds, ProtoTableModel::TimePassedInSecondsRole);
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
    return regime.m_maxTime * regime.m_repeatCount;
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
            totalTimeForOneCycleIteration += currentRegime.m_maxTime * currentRegime.m_repeatCount;
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

int RegimeManager::getTotalEstimatedTime() const
{
    int totalTime = 0;
    QSet<int> processedCycleIds;

    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        Regime regime = m_model.data(m_model.index(i, 0), ProtoTableModel::RegimeRole).value<Regime>();
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