#pragma once

#include <vector>
#include <stack>
#include <glm/glm.hpp>

struct Particle;
template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
struct VelocityGrid;

class ParticleTracker {
public:
  ParticleTracker(unsigned w, unsigned h, unsigned int ppc);

  ~ParticleTracker();

  void reinitializeParticles(OrdinalGrid<float> const* distance);
  void advect(VelocityGrid const* velocities, float dt);

  std::vector<Particle*> const* const getParticles() const;

private:

  void insertParticle(glm::vec2 pos, float radius);

  glm::vec2 jitterCoordinate(glm::vec2 coord);

  void resetParticleCount();

  static constexpr float MAX_RADUIS = 0.5f;
  static constexpr float INTERFACE_OFFSET = 1.0f;

  unsigned particlesPerCell;

  unsigned w, h;
  std::vector<Particle*> *interfaceParticles;
  std::stack<Particle*> *deadParticles;
  Grid<unsigned> *particleCount;

  // skaren ha nåra vänner eller?!
};