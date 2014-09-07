#include <bubbleTracker.h>

#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <stdlib.h>
#include <glm/ext.hpp>

BubbleTracker::BubbleTracker(unsigned w, unsigned h, unsigned d) {
  width = w; height = h; depth = d;
  bubbles = new std::vector<Bubble*>();
  deadBubbles = new std::stack<Bubble*>();
}

BubbleTracker::~BubbleTracker() {
  for (auto &b : *bubbles) {
    delete b;
  }

  delete bubbles;
  delete deadBubbles;
}

void BubbleTracker::spawnBubble(glm::vec3 p, float r, glm::vec3 v) {
  Bubble* b;

  // u not in nirvana yet?
  if (deadBubbles->size() > 0) {
    b = deadBubbles->top();
    b->position = p;
    b->radius = r;
    b->velocity = v;
    b->alive = true;
    b->id = nextBubbleId++;
    deadBubbles->pop();
  } else {
    b = new Bubble(p, r, v, nextBubbleId++);
    bubbles->push_back(b);
  }
}

void BubbleTracker::advect(
  VelocityGrid const* velocities,
  OrdinalGrid<float> *distances,
  OrdinalGrid<double> *pressures,
  glm::vec3 g,
  float dt) {
  for (auto &b : *bubbles) {
    if (!(b->alive)) {
      continue;
    }

    glm::vec3 pos = b->position;
    glm::vec3 fluidVelocity = velocities->getLerp(pos);

    // Water-Bubble force calculations
    float bubbleVolume = 3.1415*b->radius*b->radius;
    float clampedVolume = fmin(bubbleVolume, 0.3);
    float pGradX = pressures->getLerp(ceil(pos.x), pos.y, pos.z) - 
                   pressures->getLerp(floor(pos.x), pos.y, pos.z);
    float pGradY = pressures->getLerp(pos.x, ceil(pos.y), pos.z) - 
                   pressures->getLerp(pos.x, floor(pos.y), pos.z);
    float pGradZ = pressures->getLerp(pos.x, pos.y, ceil(pos.z)) - 
                   pressures->getLerp(pos.x, pos.y, floor(pos.z));
    glm::vec3 pressureGradient = glm::vec3(pGradX, pGradY, pGradZ);
    if (glm::length(pressureGradient) > 1.0f) {
      pressureGradient = glm::normalize(pressureGradient);
    }
    glm::vec3 pressureForce = - pressureGradient*K_P*clampedVolume;
    glm::vec3 viscosityForce = (fluidVelocity - b->velocity)*K_V;

    b->velocity = (1/K_V)*(K_V*fluidVelocity + pressureForce);
    b->position = b->position + b->velocity*dt;

    // kill bubbles outside fluid
    float dist = distances->getLerp(pos);
    if (dist > 0) {
      deadBubbles->push(b);
      b->alive = false;
    }

    // kill bubbles outside grid
    if (pos.x > width || pos.x < 0 || pos.y > height || pos.y < 0 || pos.z > depth || pos.z < 0) {
      deadBubbles->push(b);
      b->alive = false;
    }
  }
}

std::vector<Bubble*> const *const BubbleTracker::getBubbles() const {
  std::vector<Bubble*> *validBubbles = new std::vector<Bubble*>();
  int nAliveBubbles = bubbles->size() - deadBubbles->size();
  
  try {
    validBubbles->reserve(nAliveBubbles);
  } catch (...) {
    // tja
  }

  for (auto &b : *bubbles) {
    if (b->alive) {
      validBubbles->push_back(b);
    }
  }
  return validBubbles;
}
