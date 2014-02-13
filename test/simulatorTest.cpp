#include <gtest/gtest.h>
#include <simulator.h>
#include <state.h>
#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <iostream>
class SimulatorTest : public ::testing::Test{
protected:
  SimulatorTest() {

    readState = new State(w, h);
    writeState = new State(w, h);
    sim = new Simulator(readState, writeState);
    VelocityGrid* velocities = new VelocityGrid(w,h);

    for(int i = 0; i <= w; i++){
      for(int j = 0; j < h; j++){
        velocities->u->set(i,j,0);
      }
      velocities->u->set(i,0,1);
    }
    for(int i = 0; i < w; i++){
      for(int j = 0; j <= h; j++){
        velocities->v->set(i,j,0);
      }
    }
    velocities->u->set(1,1,1);
    readState->setVelocityGrid(velocities);
    writeState->setVelocityGrid(velocities);
    delete velocities;
  }

  ~SimulatorTest() {
    delete readState;
    delete writeState;
    delete sim;
  }
  int w=3, h=3;
  Simulator *sim;
  State *readState, *writeState;
};

TEST_F(SimulatorTest, instantiateAndDelete) {}

TEST_F(SimulatorTest, BackTrack){
  // glm::vec2 pos = sim->backTrack(readState, 1, 1, 1);
  // ASSERT_EQ(glm::vec2(0.5,1), pos);
}


TEST_F(SimulatorTest, Advect){
  sim->advect(readState, writeState, 1.0f);
  float v1 = writeState->getVelocityGrid()->u->get(1,1);
  ASSERT_EQ(v1, 0.5);
}

TEST_F(SimulatorTest, PressureGridReset) {
  OrdinalGrid<double> *p = sim->resetPressureGrid();

  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; ++i) {
      ASSERT_EQ(p->get(i,j), 0.0);
    }
  }
}
