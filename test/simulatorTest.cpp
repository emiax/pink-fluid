#include <gtest/gtest.h>
#include <simulator.h>
class SimulatorTest : public ::testing::Test{
protected:
  SimulatorTest() {
    sim = new Simulator(5, 5);
  }

  ~SimulatorTest() {
    delete sim;
  }
  Simulator *sim;
};

TEST_F(SimulatorTest, instantiateAndDelete) {}