#include <particleTracker.h>

#include <stdlib.h>
#include <particle.h>
#include <iomanip>
#include <grid.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>

ParticleTracker::ParticleTracker(unsigned w, unsigned h, unsigned int ppc) {
  interfaceParticles = new std::vector<Particle*>();
  deadParticles = new std::stack<Particle*>();
  particleCount = new Grid<unsigned>(w, h);
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


      // delete particles from full cells
    unsigned count = particleCount->get(cell);
    if (cellActive && count < particlesPerCell) {
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
          float d = distance->getLerp(pos);
          float r = (d > MAX_RADUIS) ? MAX_RADUIS : d;

          insertParticle(pos, r);
          ++count;
        }

        particleCount->set(i, j, count);
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
    
    if (p->alive) {
      glm::vec2 v = velocities->getLerp(p->position);
      glm::vec2 midPos = p->position + dt/2 * v;
      glm::vec2 midV = velocities->getLerp(midPos);
      p->position = p->position + dt * midV;
    }
  }
}

std::vector<Particle*> const *const ParticleTracker::getParticles() const {
  return interfaceParticles;
}

void ParticleTracker::insertParticle(glm::vec2 pos, float radius) {
  Particle *p;

  if (deadParticles->size() > 0) {
    p = deadParticles->top();
    p->position = pos;
    p->alive = true;
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