#pragma once
#include <functional>

class SignedDistanceFunction {
public:
  
  typedef std::function<float(const unsigned int &i, const unsigned int &j)> SignedDistFunc;

  SignedDistanceFunction(SignedDistFunc f) : sdf(f) {};
  ~SignedDistanceFunction() {};

  SignedDistFunc getFunction() {
    return sdf;
  };

  float operator()(const unsigned int &i, const unsigned int &j) {
    return sdf(i, j);
  };
  
private:
  SignedDistFunc sdf;

};