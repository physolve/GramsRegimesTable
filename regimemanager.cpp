#include "regimemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

RegimeManager::RegimeManager(QObject *parent)
    : QObject{parent}, m_model(this)
{
    loadDefaultRegimes();
    connect(&m_model, &ProtoTableModel::dataChanged, this, [this]() { setDirty(true); });
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
