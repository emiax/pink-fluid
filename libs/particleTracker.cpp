#include <particleTracker.h>

#include <stdlib.h>
#include <particle.h>
#include <iomanip>
#include <grid.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>
#include <bubbleTracker.h>
#include <glm/ext.hpp>

ParticleTracker::ParticleTracker(unsigned w, unsigned h, unsigned int ppc) {
  interfaceParticles = new std::vector<Particle*>();
  escapedParticles = new std::vector<Particle*>();
  deadParticles = new std::stack<Particle*>();
  particleCount = new Grid<unsigned>(w, h);
  corrPlus = new Grid<float>(w, h);
  corrMinus = new Grid<float>(w, h);
  particlesPerCell = ppc;
  this->w = w; this->h = h;
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

    glm::vec2 pos = p->position;
    
    // early out conditions
    if (!(p->alive)) {
      continue;
    }

    GridCoordinate cell = GridCoordinate(round(pos.x), round(pos.y));

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
      if (fabs(distance->get(i, j)) <= INTERFACE_OFFSET) {

        unsigned count = particleCount->get(i, j);

        while (count < particlesPerCell) {
          glm::vec2 pos = jitterCoordinate(glm::vec2(i, j));

          // float d = distance->getLerp(pos);
          // float r = (d > MAX_RADUIS) ? MAX_RADUIS : d;
          float r = distance->getLerp(pos);

          insertParticle(pos, r);
          ++count;
        }

        particleCount->set(i, j, count);
      }
    }
  }

  //   std::cout << "Particles: " << interfaceParticles->size() << std::endl;
//   std::cout << "Dead: " << deadParticles->size() << std::endl;

   /*   for(unsigned j = 0; j < h; ++j) {
     for(unsigned i = 0; i < w; ++i) {
       std::cout << std::setw(3) << particleCount->get(i,j);
     }
     std::cout << std::endl;
     }*/
  
  //   std::cout << std::endl;
   //   std::cin.get();
}

void ParticleTracker::advect(VelocityGrid const* velocities, float dt) {
  for (unsigned i = 0; i < interfaceParticles->size(); ++i) {
    Particle *p = interfaceParticles->at(i);

    // RK2
    if (p->alive) {
      glm::vec2 v = velocities->getLerp(p->position);
      glm::vec2 midPos = p->position + dt/2 * v;
      glm::vec2 midV = velocities->getLerp(midPos);
      p->position = p->position + dt * midV;
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

    glm::vec2 pos = p->position;
    GridCoordinate cell = GridCoordinate(round(pos.x), round(pos.y));

    float radius = fabs(p->phi);
    float dist = distance->getLerp(pos.x, pos.y);
    
    // bubble if                  (air particle) (in water)  (not touching surface)
    if (distance->isValid(cell) && p->phi > 0 && dist < 0 && fabs(dist) > radius) {
      //      std::cout << radius << std::endl;
      //      std::cin.get();
      glm::vec2 velocity = velocities->getLerp(pos);
      bt->spawnBubble(pos, radius*10.0, velocity);
    }
  }
}

void ParticleTracker::correct(OrdinalGrid<float> *distance) {

  // init correctionGrids = distanceGrid
  corrPlus->setForEach([&](unsigned i, unsigned j) {
    return distance->get(i, j);
  });
  corrMinus->setForEach([&](unsigned i, unsigned j) {
    return distance->get(i, j);
  });

  // check all particles
  for (unsigned i = 0; i < interfaceParticles->size(); ++i) {
    Particle* p = interfaceParticles->at(i);
    if (!(p->alive)) {
      continue;
    }

    glm::vec2 pos = p->position;
    GridCoordinate cell = GridCoordinate(round(pos.x), round(pos.y));

    float radius = fabs(p->phi);
    float dist = distance->getLerp(pos.x, pos.y);
    
    // if escaped                  (different signs)  (particle does not touch surface)
    if (distance->isValid(cell) && p->phi*dist < 0 && fabs(dist) > radius) {
      GridCoordinate leftUp = GridCoordinate(floor(pos.x), floor(pos.y));
      GridCoordinate rightUp = GridCoordinate(leftUp.x + 1, leftUp.y);
      GridCoordinate leftDown = GridCoordinate(leftUp.x, leftUp.y + 1);
      GridCoordinate rightDown = GridCoordinate(leftUp.x + 1, leftUp.y + 1);

      if (distance->isValid(leftUp) && 
          distance->isValid(rightUp) &&
          distance->isValid(leftDown) &&
          distance->isValid(rightDown)) {

        int sgn = p->phi > 0 ? 1 : -1;
        float leftUpContrib = sgn*(radius - glm::length(glm::vec2(leftUp) - pos));
        float rightUpContrib = sgn*(radius - glm::length(glm::vec2(rightUp) - pos));
        float leftDownContrib = sgn*(radius - glm::length(glm::vec2(leftDown) - pos));
        float rightDownContrib = sgn*(radius - glm::length(glm::vec2(rightDown) - pos));

        // air
        if (sgn == 1) {
          corrPlus->set(leftUp, glm::max(corrPlus->clampGet(leftUp), leftUpContrib));
          corrPlus->set(rightUp, glm::max(corrPlus->clampGet(rightUp), rightUpContrib));
          corrPlus->set(leftDown, glm::max(corrPlus->clampGet(leftDown), leftDownContrib));
          corrPlus->set(rightDown, glm::max(corrPlus->clampGet(rightDown), rightDownContrib));
        // fluid
        } else {
          corrMinus->set(leftUp, glm::min(corrMinus->clampGet(leftUp), leftUpContrib));
          corrMinus->set(rightUp, glm::min(corrMinus->clampGet(rightUp), rightUpContrib));
          corrMinus->set(leftDown, glm::min(corrMinus->clampGet(leftDown), leftDownContrib));
          corrMinus->set(rightDown, glm::min(corrMinus->clampGet(rightDown), rightDownContrib));
        }
      }
    }
  }

  distance->setForEach([&](unsigned i, unsigned j) {
    float p = corrPlus->get(i, j);
    float m = corrMinus->get(i, j);
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


void ParticleTracker::insertParticle(glm::vec2 pos, float radius) {
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

glm::vec2 ParticleTracker::jitterCoordinate(glm::vec2 coord) {
  return glm::vec2(
    coord.x + ((float)rand()/RAND_MAX - 0.5)*0.999999999, 
    coord.y + ((float)rand()/RAND_MAX - 0.5)*0.999999999
  );
}


void ParticleTracker::resetParticleCount() {
  particleCount->setForEach([] (unsigned i, unsigned j) {
    return 0u;
  });
}
