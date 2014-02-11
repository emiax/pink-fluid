#include <gtest/gtest.h>
#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
#include <iostream>
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
        velocities[0]->set(i,j,0);
      }
      velocities[0]->set(i,0,1);
    }
    for(int i = 0; i < w; i++){
      for(int j = 0; j <= h; j++){
        velocities[1]->set(i,j,0);
      }
    }
    velocities[0]->set(1,1,1);
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
  glm::vec2 pos = sim->backTrack(s, 1, 1, 1);
  ASSERT_EQ(glm::vec2(0.5,1), pos);
}


TEST_F(SimulatorTest, Advect){
  State *writeState = new State(w, h);
  sim->advect(s, writeState, 1);
  float v1 = (*(writeState->getVelocityGrid()))->get(1,1);
  ASSERT_EQ(v1, 0.5);

}
