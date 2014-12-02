#pragma once

#include <vector>
#include <stack>
#include <glm/glm.hpp>

struct VelocityGrid;
template<typename T>
class OrdinalGrid;

struct Bubble {
  Bubble(glm::vec3 p, float r, glm::vec3 v, int id, bool pAlive = true) {
    this->position = p;
    this->radius = r;
    this->velocity = v;
    this->alive = pAlive;
    this->id = id;
  };

  Bubble() : Bubble(glm::vec3(0.0f), 0.0, glm::vec3(0.0f), 0) {};
  Bubble(const Bubble &b) : Bubble(b.position, b.radius, b.velocity, b.id, b.alive){};
  
  ~Bubble() {};

  glm::vec3 position;
  float radius;
  glm::vec3 velocity;
  bool alive;
  int id;
};

class BubbleTracker {
public:
  BubbleTracker(unsigned w, unsigned h, unsigned d);

  ~BubbleTracker();

  void spawnBubble(glm::vec3 pos, float radius, glm::vec3 velocity);
  void advect(
    VelocityGrid const* velocities,
    OrdinalGrid<float> *distances,
    OrdinalGrid<double> *pressures,
    glm::vec3 g,
    float dt
  );
  std::vector<Bubble> getBubbles() const;
  void setBubbles(std::vector<Bubble>);
private:
  int nextBubbleId = 0;
  float width, height, depth;
  std::vector<Bubble*> *bubbles;
  std::stack<Bubble*> *deadBubbles;

  // Constants for bubble-fluid coupling
  const float K_P = 0.2f;
  const float K_V = 0.1f;
};
