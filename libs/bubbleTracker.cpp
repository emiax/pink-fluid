#include <bubbleTracker.h>

#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <stdlib.h>
#include <glm/ext.hpp>
#include <state.h>
#include <bubble.h>

BubbleTracker::BubbleTracker() {
}

BubbleTracker::~BubbleTracker() {
}

void BubbleTracker::spawnBubble(State *state, glm::vec3 p, float r, glm::vec3 v) {
  // u not in nirvana yet?
  if (state->deadBubbleIndices.size() > 0) {
    int idx = state->deadBubbleIndices.top();
    Bubble &b = state->bubbles[idx];
    b.position = p;
    b.radius = r;
    b.velocity = v;
    b.alive = true;
    b.id = state->nextBubbleId++;
    state->deadBubbleIndices.pop();
  } else {
    Bubble b = Bubble(p, r, v, state->nextBubbleId++);
    state->bubbles.push_back(b);
  }
}

void BubbleTracker::advect(State *stateFrom, State *stateTo, OrdinalGrid<double> *pressures, glm::vec3 g, float dt) {
  // copy whole bubble state to new state.
  stateTo->bubbles = stateFrom->bubbles;
  stateTo->deadBubbleIndices = stateFrom->deadBubbleIndices;
  stateTo->nextBubbleId = stateFrom->nextBubbleId;

  auto velocities = stateFrom->getVelocityGrid();
  auto sdf = stateFrom->getSignedDistanceGrid();
  
  std::vector<Bubble> &bubbles = stateTo->bubbles;
  std::stack<int> &deadBubbleIndices = stateTo->deadBubbleIndices;
  int nextBubbleId = stateTo->nextBubbleId;
  int width = stateTo->getW();
  int height = stateTo->getH();
  int depth = stateTo->getD();
  
  for (int idx = 0; idx < bubbles.size(); idx++) {
    Bubble &b = bubbles[idx];
    
    if (!(b.alive)) {
      continue;
    }

    glm::vec3 pos = b.position;
    glm::vec3 fluidVelocity = velocities->getLerp(pos);

    // Water-Bubble force calculations
    float bubbleVolume = 3.1415 * b.radius * b.radius;
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
    glm::vec3 viscosityForce = (fluidVelocity - b.velocity)*K_V;

    b.velocity = (1/K_V)*(K_V*fluidVelocity + pressureForce);
    b.position = b.position + b.velocity*dt;

    // kill bubbles outside fluid
    float dist = sdf->getLerp(pos);
    if (dist > 0) {
      deadBubbleIndices.push(idx);
      b.alive = false;
    }
    
    // kill bubbles outside grid
    if (pos.x > width || pos.x < 0 || pos.y > height || pos.y < 0 || pos.z > depth || pos.z < 0) {
      deadBubbleIndices.push(idx);
      b.alive = false;
    }
  }
}

