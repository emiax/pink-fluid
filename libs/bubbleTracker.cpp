#include <bubbleTracker.h>

#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <stdlib.h>
#include <glm/ext.hpp>

BubbleTracker::BubbleTracker(unsigned w, unsigned h) {
  width = w; height = h;
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

void BubbleTracker::spawnBubble(glm::vec2 p, float r, glm::vec2 v) {
  Bubble* b;

  // u not in nirvana yet?
  if (deadBubbles->size() > 0) {
    b = deadBubbles->top();
    b->position = p;
    b->radius = r;
    b->velocity = v;
    b->alive = true;
    deadBubbles->pop();
  } else {
    b = new Bubble(p, r, v);
    bubbles->push_back(b);
  }
}

void BubbleTracker::advect(
  VelocityGrid const* velocities,
  OrdinalGrid<float> *distances,
  OrdinalGrid<double> *pressures,
  glm::vec2 g,
  float dt) {
  for (auto &b : *bubbles) {
    if (!(b->alive)) {
      continue;
    }

    glm::vec2 pos = b->position;
    glm::vec2 fluidVelocity = velocities->getLerp(pos);

    // Water-Bubble force calculations
    float bubbleVolume = 3.1415*b->radius*b->radius;
    float clampedVolume = fmin(bubbleVolume, 0.3);
    float pGradX = pressures->getLerp(ceil(pos.x), pos.y) - pressures->getLerp(floor(pos.x), pos.y);
    float pGradY = pressures->getLerp(pos.x, ceil(pos.y)) - pressures->getLerp(pos.x, floor(pos.y));
    glm::vec2 pressureGradient = glm::vec2(pGradX, pGradY);
    if (glm::length(pressureGradient) > 1.0f) {
      pressureGradient = glm::normalize(pressureGradient);
    }
    glm::vec2 pressureForce = - pressureGradient*K_P*clampedVolume;
    glm::vec2 viscosityForce = (fluidVelocity - b->velocity)*K_V;

    b->velocity = (1/K_V)*(K_V*fluidVelocity + pressureForce);
    b->position = b->position + b->velocity*dt;

    // kill bubbles outside fluid
    float dist = distances->getLerp(pos);
    if (dist > 0) {
      deadBubbles->push(b);
      b->alive = false;
    }

    // kill bubbles outside grid
    if (pos.x > width || pos.x < 0 || pos.y > height || pos.y < 0) {
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
