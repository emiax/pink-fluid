#include <gtest/gtest.h>
#include <signedDistance.h>
#include <cmath>
#include <glm/glm.hpp>

class SignedDistanceTest : public ::testing::Test{
protected:
  SignedDistanceTest() {
    circleSignedDist = new SignedDistance([](const unsigned int &i, const unsigned int &j) {
      // distance function to circle with radius 3.0, center in (0, 0)
      return sqrt( i*i + j*j ) - 3.0f;
    });
  }

  ~SignedDistanceTest() {
    delete circleSignedDist;
  }

  SignedDistance *circleSignedDist;
};

TEST_F(SignedDistanceTest, instantiateAndDelete) {}

TEST_F(SignedDistanceTest, getSignedDistance) {
  // base case
  ASSERT_EQ( (*circleSignedDist)(0, 0), -3.0 );
  // inside
  ASSERT_FLOAT_EQ( (*circleSignedDist)(2, 2), glm::length(glm::vec2(2, 2)) - 3.0 );
  // outside
  ASSERT_FLOAT_EQ( (*circleSignedDist)(-4, -5), glm::length(glm::vec2(-4, -5)) - 3.0 );
  // on interface
  ASSERT_EQ( (*circleSignedDist)(0, 3.0), 0 );
  ASSERT_EQ( (*circleSignedDist)(3.0, 0), 0 );
}
