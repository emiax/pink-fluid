#include <particleTracker.h>

#include <stdlib.h>
#include <particle.h>
#include <iomanip>
#include <grid.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>
#include <bubbleTracker.h>
#include <glm/ext.hpp>

ParticleTracker::ParticleTracker(unsigned w, unsigned h, unsigned d, unsigned int ppc) {
  interfaceParticles = new std::vector<Particle*>();
  escapedParticles = new std::vector<Particle*>();
  deadParticles = new std::stack<Particle*>();
  particleCount = new Grid<unsigned>(w, h, d);
  corrPlus = new Grid<float>(w, h, d);
  corrMinus = new Grid<float>(w, h, d);
  particlesPerCell = ppc;
  this->w = w; this->h = h; this->d = d;
}

ParticleTracker::~ParticleTracker() {
  for (auto &p : *interfaceParticles) {
    delete p;
  }

  delete interfaceParticles;
  delete deadParticles;
  delete particleCount;
}

void ParticleTracker::reinitializeParticles(OrdinalGrid<float> const* distance) {
  resetParticleCount();

  for (unsigned i = 0; i < interfaceParticles->size(); ++i) {
    Particle* p = interfaceParticles->at(i);

    glm::vec3 pos = p->position;
    
    // early out conditions
    if (!(p->alive)) {
      continue;
    }

    GridCoordinate cell = GridCoordinate(round(pos.x), round(pos.y), round(pos.z));

    // particle outside grid?
    if (!(distance->isValid(cell))) {
      deadParticles->push(p);
      p->alive = false;
      continue;
    }
    
    bool cellActive = fabs(distance->get(cell)) < INTERFACE_OFFSET;

    // update particle radii, cell particleCount
    // and remove particles outside offset
    unsigned count = particleCount->get(cell);
    if (cellActive && count < particlesPerCell) {
      p->phi = distance->getLerp(pos);
      particleCount->set(cell, count + 1);
    } else {
      deadParticles->push(p);
      p->alive = false;
    }
  }

  // spawn new particles
  for (unsigned j = 0; j < h; ++j) {
    for (unsigned i = 0; i < w; ++i) {
      for (unsigned k = 0; k < d; ++k) {
        if (fabs(distance->get(i, j, k)) <= INTERFACE_OFFSET) {
          unsigned count = particleCount->get(i, j, k);

          while (count < particlesPerCell) {
            glm::vec3 pos = jitterCoordinate(glm::vec3(i, j, k));

            float d = distance->getLerp(pos);
            float r = (d > MAX_RADUIS) ? MAX_RADUIS : d;
            // float r = distance->getLerp(pos);

            insertParticle(pos, r);
            ++count;
          }

          particleCount->set(i, j, k, count);
        }
      }
    }
  }

  // std::cout << "Particles: " << interfaceParticles->size() << std::endl;
  // std::cout << "Dead: " << deadParticles->size() << std::endl;

  // for(unsigned j = 0; j < h; ++j) {
  //   for(unsigned i = 0; i < w; ++i) {
  //     std::cout << std::setw(3) << particleCount->get(i,j);
  //   }
  //   std::cout << std::endl;
  // }
  
  // std::cout << std::endl;
  // std::cin.get();
}

void ParticleTracker::advect(VelocityGrid const* velocities, float dt) {
  for (unsigned i = 0; i < interfaceParticles->size(); ++i) {
    Particle *p = interfaceParticles->at(i);

    // RK2
    if (p->alive) {
      glm::vec3 v = velocities->getLerp(p->position);
      glm::vec3 midPos = p->position + v*dt/2.0f;
      glm::vec3 midV = velocities->getLerp(midPos);
      p->position = p->position + dt*midV;
    }
  }
}

void ParticleTracker::feedEscaped(BubbleTracker* bt, OrdinalGrid<float> *distance, VelocityGrid const* velocities) {
  // check all particles
  for (unsigned i = 0; i < interfaceParticles->size(); ++i) {
    Particle* p = interfaceParticles->at(i);
    if (!(p->alive)) {
      continue;
    }

    glm::vec3 pos = p->position;
    GridCoordinate cell = GridCoordinate(round(pos.x), round(pos.y), round(pos.z));

    float radius = fabs(p->phi);
    float dist = distance->getLerp(pos.x, pos.y, pos.z);
    
    // bubble if                  (air particle) (in water)  (not touching surface)
    if (distance->isValid(cell) && p->phi > 0 && dist < 0 && fabs(dist) > radius && radius > 0.04) {
      glm::vec3 velocity = velocities->getLerp(pos);
      bt->spawnBubble(pos, radius*10.0, velocity);
    }
  }
}

void ParticleTracker::correct(OrdinalGrid<float> *distance) {

  // init correctionGrids = distanceGrid
  corrPlus->setForEach([&](unsigned i, unsigned j, unsigned k) {
    return distance->get(i, j, k);
  });
  corrMinus->setForEach([&](unsigned i, unsigned j, unsigned k) {
    return distance->get(i, j, k);
  });

  // check all particles
  for (unsigned i = 0; i < interfaceParticles->size(); ++i) {
    Particle* p = interfaceParticles->at(i);
    if (!(p->alive)) {
      continue;
    }

    glm::vec3 pos = p->position;
    GridCoordinate cell = GridCoordinate(round(pos.x), round(pos.y), round(pos.z));

    float radius = fabs(p->phi);
    float dist = distance->getLerp(pos.x, pos.y, pos.z);
    
    // if escaped                  (different signs)  (particle does not touch surface)
    if (distance->isValid(cell) && p->phi*dist < 0 && fabs(dist) > radius) {
      GridCoordinate leftUpFront = GridCoordinate(floor(pos.x), floor(pos.y), floor(pos.z));
      GridCoordinate rightUpFront = GridCoordinate(leftUpFront.x + 1, leftUpFront.y, leftUpFront.z);
      GridCoordinate leftDownFront = GridCoordinate(leftUpFront.x, leftUpFront.y + 1, leftUpFront.z);
      GridCoordinate rightDownFront = GridCoordinate(leftUpFront.x + 1, leftUpFront.y + 1, leftUpFront.z);

      GridCoordinate leftUpBack = GridCoordinate(floor(pos.x), floor(pos.y), floor(pos.z) + 1);
      GridCoordinate rightUpBack = GridCoordinate(leftUpBack.x + 1, leftUpBack.y, leftUpBack.z);
      GridCoordinate leftDownBack = GridCoordinate(leftUpBack.x, leftUpBack.y + 1, leftUpBack.z);
      GridCoordinate rightDownBack = GridCoordinate(leftUpBack.x + 1, leftUpBack.y + 1, leftUpBack.z);

      if (distance->isValid(leftUpFront) && 
          distance->isValid(rightUpFront) &&
          distance->isValid(leftDownFront) &&
          distance->isValid(rightDownFront) &&
          distance->isValid(leftUpBack) && 
          distance->isValid(rightUpBack) &&
          distance->isValid(leftDownBack) &&
          distance->isValid(rightDownBack)) {

        int sgn = p->phi > 0 ? 1 : -1;
        float leftUpFrontContrib = sgn*(radius - glm::length(glm::vec3(leftUpFront) - pos));
        float rightUpFrontContrib = sgn*(radius - glm::length(glm::vec3(rightUpFront) - pos));
        float leftDownFrontContrib = sgn*(radius - glm::length(glm::vec3(leftDownFront) - pos));
        float rightDownFrontContrib = sgn*(radius - glm::length(glm::vec3(rightDownFront) - pos));

        float leftUpBackContrib = sgn*(radius - glm::length(glm::vec3(leftUpBack) - pos));
        float rightUpBackContrib = sgn*(radius - glm::length(glm::vec3(rightUpBack) - pos));
        float leftDownBackContrib = sgn*(radius - glm::length(glm::vec3(leftDownBack) - pos));
        float rightDownBackContrib = sgn*(radius - glm::length(glm::vec3(rightDownBack) - pos));

        // air
        if (sgn == 1) {
          corrPlus->set(leftUpFront, glm::max(corrPlus->clampGet(leftUpFront), leftUpFrontContrib));
          corrPlus->set(rightUpFront, glm::max(corrPlus->clampGet(rightUpFront), rightUpFrontContrib));
          corrPlus->set(leftDownFront, glm::max(corrPlus->clampGet(leftDownFront), leftDownFrontContrib));
          corrPlus->set(rightDownFront, glm::max(corrPlus->clampGet(rightDownFront), rightDownFrontContrib));

          corrPlus->set(leftUpBack, glm::max(corrPlus->clampGet(leftUpBack), leftUpBackContrib));
          corrPlus->set(rightUpBack, glm::max(corrPlus->clampGet(rightUpBack), rightUpBackContrib));
          corrPlus->set(leftDownBack, glm::max(corrPlus->clampGet(leftDownBack), leftDownBackContrib));
          corrPlus->set(rightDownBack, glm::max(corrPlus->clampGet(rightDownBack), rightDownBackContrib));
        // fluid
        } else {
          corrMinus->set(leftUpFront, glm::min(corrMinus->clampGet(leftUpFront), leftUpFrontContrib));
          corrMinus->set(rightUpFront, glm::min(corrMinus->clampGet(rightUpFront), rightUpFrontContrib));
          corrMinus->set(leftDownFront, glm::min(corrMinus->clampGet(leftDownFront), leftDownFrontContrib));
          corrMinus->set(rightDownFront, glm::min(corrMinus->clampGet(rightDownFront), rightDownFrontContrib));

          corrMinus->set(leftUpBack, glm::min(corrMinus->clampGet(leftUpBack), leftUpBackContrib));
          corrMinus->set(rightUpBack, glm::min(corrMinus->clampGet(rightUpBack), rightUpBackContrib));
          corrMinus->set(leftDownBack, glm::min(corrMinus->clampGet(leftDownBack), leftDownBackContrib));
          corrMinus->set(rightDownBack, glm::min(corrMinus->clampGet(rightDownBack), rightDownBackContrib));
        }
      }
    }
  }

  distance->setForEach([&](unsigned i, unsigned j, unsigned k) {
    float p = corrPlus->get(i, j, k);
    float m = corrMinus->get(i, j, k);
    return fabs(m) < fabs(p) ? m : p;
  });
}

std::vector<Particle> ParticleTracker::getParticles() const {
  std::vector<Particle> validParticles;
  int nAliveParticles = interfaceParticles->size() - deadParticles->size();
  validParticles.reserve(nAliveParticles);

  for (int i = 0; i < interfaceParticles->size(); i++) {
    Particle p = *interfaceParticles->at(i);
    if (p.alive) {
      validParticles.push_back(p);
    }
  }
  return validParticles;
}


void ParticleTracker::insertParticle(glm::vec3 pos, float radius) {
  Particle *p;

  if (deadParticles->size() > 0) {
    p = deadParticles->top();
    p->position = pos;
    p->phi = radius;
    p->alive = true;
    p->phi = radius;
    deadParticles->pop();
  } else {
    p = new Particle(pos, radius);
    interfaceParticles->push_back(p);
  }
}

glm::vec3 ParticleTracker::jitterCoordinate(glm::vec3 coord) {
  return glm::vec3(
    coord.x + ((float)rand()/RAND_MAX - 0.5)*0.999999999, 
    coord.y + ((float)rand()/RAND_MAX - 0.5)*0.999999999,
    coord.z + ((float)rand()/RAND_MAX - 0.5)*0.999999999
  );
}


void ParticleTracker::resetParticleCount() {
  particleCount->setForEach([] (unsigned i, unsigned j, unsigned k) {
    return 0u;
  });
}
