#pragma once

#include <vector>
#include <stack>
#include <glm/glm.hpp>

struct VelocityGrid;
class State;

template<typename T>
class OrdinalGrid;


class BubbleTracker {
public:
  BubbleTracker();

  ~BubbleTracker();

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
