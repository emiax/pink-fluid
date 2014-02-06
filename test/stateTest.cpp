#include <gtest/gtest.h>
#include <state.h>

class StateTest : public ::testing::Test{
protected:
  StateTest() {
  }

  ~StateTest() {
  }
};

TEST_F(StateTest, instantiateAndDelete) {
  // State *state = new State(5, 5);
  // delete state;
}