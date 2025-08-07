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

TEST_F(RegimeManagerTest, SaveRegimesAs) {
    RegimeManager manager;
    QList<Regime> regimes;
    Regime r;
    r.m_name = "Save As Test";
    regimes.append(r);
    manager.model()->setRegimes(regimes);

    QTemporaryFile file(tempDir.path() + "/save_as_test.json");
    ASSERT_TRUE(file.open());
    file.close();

    manager.saveRegimesAs(QUrl::fromLocalFile(file.fileName()));

    QFile savedFile(file.fileName());
    ASSERT_TRUE(savedFile.open(QIODevice::ReadOnly));
    QByteArray data = savedFile.readAll();
    ASSERT_TRUE(data.contains("Save As Test"));
    ASSERT_EQ(manager.currentFilePath(), QUrl::fromLocalFile(file.fileName()));
}
