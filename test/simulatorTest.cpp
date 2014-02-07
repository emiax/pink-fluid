#include <gtest/gtest.h>
#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
class SimulatorTest : public ::testing::Test{
protected:
  SimulatorTest() {

    sim = new Simulator(w, h);
    s = new State(w, h);
    OrdinalGrid<float>** velocities = new OrdinalGrid<float>*[2];
    velocities[0] = new OrdinalGrid<float>(w+1, h);
    velocities[1] = new OrdinalGrid<float>(w, h+1);
    for(int i = 0; i <= w; i++){
      for(int j = 0; j < h; j++){
        velocities[0]->set(i,j,i);
      }
    }
    for(int i = 0; i < w; i++){
      for(int j = 0; j <= h; j++){
        velocities[1]->set(i,j,0);
      }
    }
    s->setVelocityGrid(velocities);
  }

  ~SimulatorTest() {
    delete sim;
    delete s;
  }
  int w=3, h=3;
  Simulator *sim;
  State *s;
};

TEST_F(SimulatorTest, instantiateAndDelete) {}

TEST_F(SimulatorTest, BackTrack){
  glm::vec2 pos = sim->backTrack(s, 2, 0, 1);
  ASSERT_EQ(pos, glm::vec2(1,0));
}


TEST_F(SimulatorTest, Advect){
  State *writeState = new State(w, h);
  sim->advect(s, writeState, 1);
  float v = (*(writeState->getVelocityGrid()))->get(2,0);
  ASSERT_EQ(v, 1);
}
