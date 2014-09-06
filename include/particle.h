#pragma once

#include <glm/glm.hpp>

struct Particle {
  
  Particle(float x, float y, float z, float p) {
    this->position = glm::vec3(x, y, z);
    this->phi = p;
    alive = true;
  };

  Particle(glm::vec3 pos, float p) : Particle(pos.x, pos.y, pos.z, p) {};

  ~Particle() {};

  glm::vec3 position;
  float phi;
  bool alive;
};