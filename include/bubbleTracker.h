#pragma once

#include <vector>
#include <stack>
#include <glm/glm.hpp>

struct VelocityGrid;
template<typename T>
class OrdinalGrid;

struct Bubble {
  Bubble(glm::vec2 p, float r, glm::vec2 v) {
    this->position = p;
    this->radius = r;
    this->velocity = v;
    this->alive = true;
  };

  ~Bubble() {};

  glm::vec2 position;
  float radius;
  glm::vec2 velocity;
  bool alive;
};

class BubbleTracker {
public:
  BubbleTracker(unsigned w, unsigned h);

  ~BubbleTracker();

  void spawnBubble(glm::vec2 pos, float radius, glm::vec2 velocity);
  void advect(
    VelocityGrid const* velocities,
    OrdinalGrid<float> *distances,
    OrdinalGrid<double> *pressures,
    glm::vec2 g,
    float dt
  );
  std::vector<Bubble*> const *const getBubbles() const;

private:
  float width, height;
  std::vector<Bubble*> *bubbles;
  std::stack<Bubble*> *deadBubbles;

  // density in kg/m³
  const float AIR_DENSITY = 1.2041f;
  const float WATER_DENSITY = 1000.0f;

  // Constants for bubble-fluid coupling
  const float K_P = 0.2f;
  const float K_V = 0.1f;
};