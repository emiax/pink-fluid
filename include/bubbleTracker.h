#pragma once

#include <vector>
#include <stack>
#include <glm/glm.hpp>
#include <bubble.h>

struct VelocityGrid;
class State;

template<typename T>
class OrdinalGrid;


class BubbleTracker {
public:
  BubbleTracker();

  ~BubbleTracker();

  void killBubblesOutsideFluid(State *state);
  void addBubblesInsideFluid(State *state, std::vector<Bubble> &bubbles);
  void spawnBubble(State *state, glm::vec3 pos, float radius, glm::vec3 velocity);
  /*  void advect(
    VelocityGrid const* velocities,
    OrdinalGrid<float> *distances,
    OrdinalGrid<double> *pressures,
    glm::vec3 g,
    float dt
    );*/
  void advect(State *stateFrom, State *stateTo, OrdinalGrid<double> *pressures, glm::vec3 g, float dt);
  
private:


  // Constants for bubble-fluid coupling
  const float K_P = 0.2f;
  const float K_V = 0.1f;
};
