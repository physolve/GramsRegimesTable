#include <gtest/gtest.h>
#include "regimemanager.h"

TEST(TimeCalculations, GetTotalTimeForRegime)
{
    RegimeManager manager;
    manager.model()->clear();
    manager.model()->addRow("Test Regime");
    manager.model()->setData(manager.model()->index(0, 0), 10, ProtoTableModel::MaxTimeRole);
    manager.model()->setData(manager.model()->index(0, 0), 5, ProtoTableModel::RepeatRole);

    ASSERT_EQ(manager.getTotalTimeForRegime(0), 50);
}

TEST(TimeCalculations, GetElapsedTimeForRegime)
{
    RegimeManager manager;
    manager.model()->clear();
    manager.model()->addRow("Test Regime");
    manager.model()->setData(manager.model()->index(0, 0), 10, ProtoTableModel::TimePassedInSecondsRole);

    ASSERT_EQ(manager.getElapsedTimeForRegime(0), 10);
}

TEST(TimeCalculations, GetTotalTimeForCycle)
{
    RegimeManager manager;
    manager.model()->clear();
    manager.model()->addRow("Regime 1");
    manager.model()->setData(manager.model()->index(0, 0), 10, ProtoTableModel::MaxTimeRole);
    manager.model()->setData(manager.model()->index(0, 0), 2, ProtoTableModel::RepeatRole);
    manager.model()->addRow("Regime 2");
    manager.model()->setData(manager.model()->index(1, 0), 5, ProtoTableModel::MaxTimeRole);
    manager.model()->setData(manager.model()->index(1, 0), 3, ProtoTableModel::RepeatRole);
    manager.model()->groupRows({0, 1});

    ASSERT_EQ(manager.getTotalTimeForCycle(0), 35);
}

TEST(TimeCalculations, GetElapsedTimeForCycle)
{
    RegimeManager manager;
    manager.model()->clear();
    manager.model()->addRow("Regime 1");
    manager.model()->setData(manager.model()->index(0, 0), 5, ProtoTableModel::TimePassedInSecondsRole);
    manager.model()->addRow("Regime 2");
    manager.model()->setData(manager.model()->index(1, 0), 3, ProtoTableModel::TimePassedInSecondsRole);
    manager.model()->groupRows({0, 1});

    ASSERT_EQ(manager.getElapsedTimeForCycle(0), 8);
}

TEST(TimeCalculations, GetTotalEstimatedTime)
{
    RegimeManager manager;
    manager.model()->clear();
    manager.model()->addRow("Regime 1");
    manager.model()->setData(manager.model()->index(0, 0), 10, ProtoTableModel::MaxTimeRole);
    manager.model()->setData(manager.model()->index(0, 0), 2, ProtoTableModel::RepeatRole);
    manager.model()->addRow("Regime 2");
    manager.model()->setData(manager.model()->index(1, 0), 5, ProtoTableModel::MaxTimeRole);
    manager.model()->setData(manager.model()->index(1, 0), 3, ProtoTableModel::RepeatRole);

    ASSERT_EQ(manager.getTotalEstimatedTime(), 35);
}

TEST(TimeCalculations, GetTotalElapsedTime)
{
    RegimeManager manager;
    manager.model()->clear();
    manager.model()->addRow("Regime 1");
    manager.model()->setData(manager.model()->index(0, 0), 5, ProtoTableModel::TimePassedInSecondsRole);
    manager.model()->addRow("Regime 2");
    manager.model()->setData(manager.model()->index(1, 0), 3, ProtoTableModel::TimePassedInSecondsRole);

    ASSERT_EQ(manager.getTotalElapsedTime(), 8);
}
