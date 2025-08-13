#include <gtest/gtest.h>
#include "regimemanager.h"
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include <gtest/gtest.h>
#include "regimemanager.h"
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>

class RegimeManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        ASSERT_TRUE(tempDir.isValid());
        originalPath = QDir::currentPath();
        QDir::setCurrent(tempDir.path());

        QDir dir(tempDir.path());
        dir.mkdir("profile");
        QFile dummyFile(dir.filePath("profile/regime_a.json"));
        ASSERT_TRUE(dummyFile.open(QIODevice::WriteOnly));
        dummyFile.write("[{\"name\": \"Test Regime\", \"condition\": {\"type\": \"temp\", \"temp\": 100, \"time\": 0}, \"repeat\": {\"count\": 1}, \"max_time\": 0, \"cycle\": null}]");
        dummyFile.close();
    }

    void TearDown() override {
        // The temporary directory and its contents are removed automatically
        QDir::setCurrent(originalPath);
    }

    QTemporaryDir tempDir;
    QString originalPath;
};


TEST_F(RegimeManagerTest, LoadDefaultRegimes) {
    RegimeManager manager;
    manager.loadDefaultRegimes();
    ProtoTableModel* model = manager.model();
    ASSERT_GT(model->rowCount(), 0);
}

TEST_F(RegimeManagerTest, ImportRegimes) {
    RegimeManager manager;
    QTemporaryFile file(tempDir.path() + "/test.json");
    ASSERT_TRUE(file.open());
    QTextStream out(&file);
    out << "[{\"name\": \"Test Regime\", \"condition\": {\"type\": \"temp\", \"temp\": 100, \"time\": 0}, \"repeat\": {\"count\": 1}, \"max_time\": 0, \"cycle\": null}]";
    file.close();

    manager.importRegimes(QUrl::fromLocalFile(file.fileName()));
    ProtoTableModel* model = manager.model();
    ASSERT_EQ(model->rowCount(), 1);
    ASSERT_EQ(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QString("Test Regime"));
}

TEST_F(RegimeManagerTest, ExportRegimes) {
    RegimeManager manager;
    QList<Regime> regimes;
    Regime r;
    r.m_name = "Export Test";
    regimes.append(r);
    manager.model()->setRegimes(regimes);

    QTemporaryFile file(tempDir.path() + "/export_test.json");
    ASSERT_TRUE(file.open());
    file.close();

    manager.exportRegimes(QUrl::fromLocalFile(file.fileName()));

    QFile exportedFile(file.fileName());
    ASSERT_TRUE(exportedFile.open(QIODevice::ReadOnly));
    QByteArray data = exportedFile.readAll();
    ASSERT_TRUE(data.contains("Export Test"));
}

TEST_F(RegimeManagerTest, SaveRegimes) {
    RegimeManager manager;
    QList<Regime> regimes;
    Regime r;
    r.m_name = "Save Test";
    regimes.append(r);
    manager.model()->setRegimes(regimes);

    QTemporaryFile file(tempDir.path() + "/save_test.json");
    ASSERT_TRUE(file.open());
    file.close();

    manager.setCurrentFilePath(QUrl::fromLocalFile(file.fileName()));
    manager.saveRegimes();

    QFile savedFile(file.fileName());
    ASSERT_TRUE(savedFile.open(QIODevice::ReadOnly));
    QByteArray data = savedFile.readAll();
    ASSERT_TRUE(data.contains("Save Test"));
}

TEST_F(RegimeManagerTest, ExternalModuleAPI) {
    RegimeManager manager;
    QList<Regime> regimes;
    
    // Create a regime with time condition and 2 repeats
    Regime r1;
    r1.m_name = "API Test Regime";
    r1.m_maxTime = 60; // 60 seconds execution time
    r1.m_repeatCount = 2;
    r1.m_condition.type = "time";
    r1.m_condition.time = 1; // 1 minute (60 seconds) condition time
    regimes.append(r1);
    
    manager.model()->setRegimes(regimes);
    
    // Test 1: Start regime execution
    ASSERT_TRUE(manager.startRegimeExecution(0));
    auto info = manager.getRegimeExecutionInfo(0);
    ASSERT_EQ(info["currentRepeat"].toInt(), 0);
    ASSERT_FALSE(info["conditionCompleted"].toBool());
    
    // Test 2: Update condition progress
    ASSERT_TRUE(manager.updateConditionProgress(0, 30, 0)); // 30 seconds into condition
    info = manager.getRegimeExecutionInfo(0);
    ASSERT_EQ(info["conditionTimePassed"].toInt(), 30);
    
    // Test 3: Complete condition
    ASSERT_TRUE(manager.confirmConditionCompletion(0, 0));
    info = manager.getRegimeExecutionInfo(0);
    ASSERT_TRUE(info["conditionCompleted"].toBool());
    ASSERT_EQ(info["conditionTimePassed"].toInt(), 60); // Full condition time
    
    // Test 4: Update regime progress
    ASSERT_TRUE(manager.updateRegimeProgress(0, 45, 0)); // 45 seconds into execution
    info = manager.getRegimeExecutionInfo(0);
    ASSERT_EQ(info["regimeTimePassed"].toInt(), 45);
    
    // Test 5: Complete first repeat
    ASSERT_TRUE(manager.completeCurrentRepeat(0, 0));
    info = manager.getRegimeExecutionInfo(0);
    ASSERT_EQ(info["currentRepeat"].toInt(), 1); // Moved to repeat 1
    ASSERT_EQ(info["repeatsDone"].toInt(), 1);
    ASSERT_FALSE(info["conditionCompleted"].toBool()); // Reset for new repeat
    ASSERT_EQ(info["conditionTimePassed"].toInt(), 0); // Reset
    ASSERT_EQ(info["regimeTimePassed"].toInt(), 0); // Reset
    
    // Test 6: Skip second repeat
    ASSERT_TRUE(manager.skipCurrentRepeat(0, 1));
    info = manager.getRegimeExecutionInfo(0);
    ASSERT_EQ(info["repeatsSkipped"].toInt(), 1);
    // Should be done since all repeats processed (1 done + 1 skipped = 2 total)
    auto state = manager.model()->data(manager.model()->index(0, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    ASSERT_EQ(state, RegimeEnums::State::Done);
    
    // Test 7: Reset regime
    ASSERT_TRUE(manager.resetRegimeExecution(0));
    info = manager.getRegimeExecutionInfo(0);
    ASSERT_EQ(info["currentRepeat"].toInt(), 0);
    ASSERT_EQ(info["repeatsDone"].toInt(), 0);
    ASSERT_EQ(info["repeatsSkipped"].toInt(), 0);
    state = manager.model()->data(manager.model()->index(0, 0), ProtoTableModel::StateRole).value<RegimeEnums::State>();
    ASSERT_EQ(state, RegimeEnums::State::Waiting);
}

TEST_F(RegimeManagerTest, APIValidation) {
    RegimeManager manager;
    QList<Regime> regimes;
    
    Regime r1;
    r1.m_name = "Validation Test";
    r1.m_maxTime = 60;
    r1.m_repeatCount = 1;
    r1.m_condition.type = "none";
    regimes.append(r1);
    
    manager.model()->setRegimes(regimes);
    
    // Test invalid regime ID
    ASSERT_FALSE(manager.startRegimeExecution(-1));
    ASSERT_FALSE(manager.startRegimeExecution(999));
    
    // Start regime
    ASSERT_TRUE(manager.startRegimeExecution(0));
    
    // Test repeat validation
    ASSERT_FALSE(manager.updateRegimeProgress(0, 30, 999)); // Wrong repeat number
    ASSERT_TRUE(manager.updateRegimeProgress(0, 30, 0));    // Correct repeat number
    
    // Test invalid time values
    ASSERT_FALSE(manager.updateRegimeProgress(0, -10, 0));  // Negative time
    ASSERT_FALSE(manager.updateRegimeProgress(0, 999, 0));  // Exceeds limit
    
    // Test double start
    ASSERT_FALSE(manager.startRegimeExecution(0)); // Already running
}
