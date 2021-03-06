#include <gtest/gtest.h>
#include <signedDistanceFunction.h>
#include <cmath>
#include <glm/glm.hpp>

class SignedDistanceFunctionTest : public ::testing::Test{
protected:
  SignedDistanceFunctionTest() {
    circleSignedDist = new SignedDistanceFunction([](const unsigned int &i, const unsigned int &j, const unsigned int &k) {
      // distance function to sphere with radius 3.0, center in (0, 0)
      return sqrt( i*i + j*j + k*k) - 3.0f;
    });
  }

  ~SignedDistanceFunctionTest() {
    delete circleSignedDist;
  }

  SignedDistanceFunction *circleSignedDist;
};

TEST_F(SignedDistanceFunctionTest, instantiateAndDelete) {}

TEST_F(SignedDistanceFunctionTest, getSignedDistance) {
  // base case
  ASSERT_EQ( (*circleSignedDist)(0, 0, 0), -3.0 );
  // inside
  ASSERT_FLOAT_EQ( (*circleSignedDist)(2, 2, 2), glm::length(glm::vec3(2, 2, 2)) - 3.0 );
  // outside
  ASSERT_FLOAT_EQ( (*circleSignedDist)(-4, -5, 2), glm::length(glm::vec3(-4, -5, 2)) - 3.0 );
  // on interface
  ASSERT_EQ( (*circleSignedDist)(0, 3.0, 0), 0 );
  ASSERT_EQ( (*circleSignedDist)(3.0, 0, 0), 0 );
}
