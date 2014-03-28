#pragma once

#include <glm/glm.hpp>

struct Particle {
  
  Particle(float x, float y, float p) {
    this->position = glm::vec2(x, y);
    this->phi = p;
    alive = true;
  };

  Particle(glm::vec2 pos, float p) : Particle(pos.x, pos.y, p) {};

  ~Particle() {};

  glm::vec2 position;
  float phi;
  bool alive;
};