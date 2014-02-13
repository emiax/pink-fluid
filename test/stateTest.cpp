#include <gtest/gtest.h>
#include <state.h>

class StateTest : public ::testing::Test{
protected:
  StateTest() {
    state = new State(5, 5);
  }

  ~StateTest() {
    delete state;
  }
  State *state;
};

TEST_F(StateTest, instantiateAndDelete) {}
