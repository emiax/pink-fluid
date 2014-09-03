#include <bubbleTracker.h>

#include <velocityGrid.h>
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

  // you not in nirvana yet?
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

void BubbleTracker::advect(VelocityGrid const* velocities, glm::vec2 g, float dt) {
  for (auto &b : *bubbles) {
    if (!(b->alive)) {
      continue;
    }

    glm::vec2 pos = b->position;

    glm::vec2 fluidVelocity = velocities->getLerp(pos);

    b->velocity = b->velocity + fluidVelocity - g;
    b->position = b->position + b->velocity*dt;

    // kill bubbles outside grid
    if (pos.x > width || pos.x < 0 || pos.y > height || pos.y < 0) {
      deadBubbles->push(b);
      b->alive = false;
    }
  }
}

std::vector<Bubble*> const *const BubbleTracker::getBubbles() const {
  return bubbles;
}
