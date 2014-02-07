#include <gtest/gtest.h>
#include <ordinalGrid.h>
#include <glm/glm.hpp>

class OrdinalGridTest : public ::testing::Test{
protected:
  OrdinalGridTest() {
    doubleGrid = new OrdinalGrid<double>(10, 10);
    vec3Grid = new OrdinalGrid<glm::vec3>(10,10);
  }

  ~OrdinalGridTest() {
    delete doubleGrid;
    delete vec3Grid;
  }
  OrdinalGrid<glm::vec3> *vec3Grid;
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

TEST_F(OrdinalGridTest, setAndGetVec3){
  vec3Grid->set(4, 3, glm::vec3(1));
  vec3Grid->set(4, 4, glm::vec3(2));
  vec3Grid->set(5, 3, glm::vec3(3));
  vec3Grid->set(5, 4, glm::vec3(4));
  {
    glm::vec3 v = vec3Grid->getInterpolated(4.0, 3.5);
    EXPECT_EQ(v, glm::vec3(1.5));
  }

  {
    glm::vec3 v = vec3Grid->getInterpolated(4.5, 4.0);
    EXPECT_EQ(v, glm::vec3(3.0));
  }

  {
    glm::vec3 v = vec3Grid->getInterpolated(4.5, 3.5);
    EXPECT_EQ(v, glm::vec3(2.5));
  }
}
