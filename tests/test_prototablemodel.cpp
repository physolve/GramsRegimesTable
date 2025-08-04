#include <gtest/gtest.h>
#include "prototablemodel.h"

TEST(ProtoTableModelTest, AddRow) {
    ProtoTableModel model;
    model.clear();
    int initialRowCount = model.rowCount();
    model.addRow("Test Regime");
    ASSERT_EQ(model.rowCount(), initialRowCount + 1);
}

TEST(ProtoTableModelTest, DeleteRow) {
    ProtoTableModel model;
    model.clear();
    model.addRow("Test Regime");
    int initialRowCount = model.rowCount();
    QVariantList rowsToDelete;
    rowsToDelete.append(0);
    model.deleteRows(rowsToDelete);
    ASSERT_EQ(model.rowCount(), initialRowCount - 1);
}

TEST(ProtoTableModelTest, GroupRows) {
    ProtoTableModel model;
    model.clear();
    model.addRow("Test Regime 1");
    model.addRow("Test Regime 2");
    QVariantList rowsToGroup;
    rowsToGroup.append(0);
    rowsToGroup.append(1);
    model.groupRows(rowsToGroup);
    ASSERT_EQ(model.data(model.index(0, 0), ProtoTableModel::CycleStatusRole).toInt(), 1);
    ASSERT_EQ(model.data(model.index(1, 0), ProtoTableModel::CycleStatusRole).toInt(), 2);
}

TEST(ProtoTableModelTest, UngroupRows) {
    ProtoTableModel model;
    model.clear();
    model.addRow("Test Regime 1");
    model.addRow("Test Regime 2");
    QVariantList rowsToGroup;
    rowsToGroup.append(0);
    rowsToGroup.append(1);
    model.groupRows(rowsToGroup);
    QVariantList rowsToUngroup;
    rowsToUngroup.append(0);
    model.ungroupRows(rowsToUngroup);
    ASSERT_EQ(model.data(model.index(0, 0), ProtoTableModel::CycleStatusRole).toInt(), 0);
    ASSERT_EQ(model.data(model.index(1, 0), ProtoTableModel::CycleStatusRole).toInt(), 0);
}

TEST(ProtoTableModelTest, MoveRows) {
    ProtoTableModel model;
    model.clear();
    model.addRow("Test Regime 1");
    model.addRow("Test Regime 2");
    model.addRow("Test Regime 3");
    model.moveRows(QModelIndex(), 0, 1, QModelIndex(), 2);
    ASSERT_EQ(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QString("Test Regime 2"));
    ASSERT_EQ(model.data(model.index(1, 0), Qt::DisplayRole).toString(), QString("Test Regime 1"));
    ASSERT_EQ(model.data(model.index(2, 0), Qt::DisplayRole).toString(), QString("Test Regime 3"));
}

TEST(ProtoTableModelTest, DeleteRowsWithCycle) {
    ProtoTableModel model;
    model.clear();
    model.addRow("Test Regime 1");
    model.addRow("Test Regime 2");
    model.addRow("Test Regime 3");
    QVariantList rowsToGroup;
    rowsToGroup.append(0);
    rowsToGroup.append(1);
    model.groupRows(rowsToGroup);

    int initialRowCount = model.rowCount();
    QVariantList rowsToDelete;
    rowsToDelete.append(0);
    model.deleteRows(rowsToDelete);
    ASSERT_EQ(model.rowCount(), initialRowCount - 2);
}