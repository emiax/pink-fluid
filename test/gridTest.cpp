#include <gtest/gtest.h>
#include <grid.h>
class GridTest : public ::testing::Test{
protected:
  GridTest() {
    doubleGrid = new Grid<double>(10, 10);
  }

  ~GridTest() {
    delete doubleGrid;
  }

  Grid<double> *doubleGrid;
};

TEST_F(GridTest, gettingAndSetting){
  double a = 1.0;
  doubleGrid->set(4, 3, a);
  double b = doubleGrid->get(4, 3);
  ASSERT_EQ(a, b);
}

