#include <gtest/gtest.h>
#include "prototablemodel.h"
#include "regimemanager.h"

TEST(ProtoTableModelTest, SetRegimes) {
    ProtoTableModel model;
    QList<Regime> regimes;
    Regime r1;
    r1.m_name = "Test Regime 1";
    regimes.append(r1);
    Regime r2;
    r2.m_name = "Test Regime 2";
    regimes.append(r2);
    model.setRegimes(regimes);
    ASSERT_EQ(model.rowCount(), 2);
    ASSERT_EQ(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QString("Test Regime 1"));
}