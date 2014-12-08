#pragma once

#include <glm/glm.hpp>


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
