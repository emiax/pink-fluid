#pragma once
#include <functional>

class SignedDistance {
public:
  
  typedef std::function<float(const unsigned int &i, const unsigned int &j)> SignedDistFunc;

  SignedDistance(SignedDistFunc f) : sdf(f) {};
  ~SignedDistance() {};

  float operator()(const unsigned int &i, const unsigned int &j) {
    return sdf(i, j);
  };
  
private:
  SignedDistFunc sdf;

};