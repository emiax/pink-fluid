#include <gtest/gtest.h>
#include <ordinalGrid.h>
class OrdinalGridTest : public ::testing::Test{
protected:
  OrdinalGridTest() {
    doubleGrid = new OrdinalGrid<double>(10, 10);
  }

  ~OrdinalGridTest() {
    delete doubleGrid;
  }

  OrdinalGrid<double> *doubleGrid;
};

TEST_F(OrdinalGridTest, settingAndGetting) {
  double a = 1.0;
  doubleGrid->set(2, 5, a);
  double b = doubleGrid->get(2, 5);
  ASSERT_EQ(a, b);
}

TEST_F(OrdinalGridTest, settingAndGettingInterpolated) {
  doubleGrid->set(4, 3, 1);
  doubleGrid->set(4, 4, 2);
  doubleGrid->set(5, 3, 3);
  doubleGrid->set(5, 4, 4);

  {
    double v = doubleGrid->getInterpolated(4.0, 3.5);
    EXPECT_EQ(v, 1.5);
  }

  {
    double v = doubleGrid->getInterpolated(4.5, 4.0);
    EXPECT_EQ(v, 3.0);
  }

  {
    double v = doubleGrid->getInterpolated(4.5, 3.5);
    EXPECT_EQ(v, 2.5);
  }
}
