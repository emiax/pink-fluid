/*#include <gtest/gtest.h>
#include <velocityGrid.h>

class VelocityGridTest : public ::testing::Test{
protected:
  VelocityGridTest() {
    velocityGrid = new VelocityGrid(5,5);
  }

  ~VelocityGridTest() {
    delete velocityGrid;
  }
  VelocityGrid *velocityGrid;
};

TEST_F(VelocityGridTest, instantiateAndDelete) {}

TEST_F(VelocityGridTest, getCell){
  velocityGrid->u->set(0,0,1.0f);
  velocityGrid->v->set(0,0,1.0f);
  ASSERT_EQ(glm::vec2(0.5,0.5), velocityGrid->getCell(0,0));
}
*/
