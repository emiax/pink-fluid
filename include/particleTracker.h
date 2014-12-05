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
class BubbleTracker;
class State;

class ParticleTracker {
public:
  ParticleTracker(unsigned w, unsigned h, unsigned d, unsigned int ppc);

  ~ParticleTracker();

  void reinitializeParticles(OrdinalGrid<float> const* distance);
  void advect(VelocityGrid const* velocities, float dt);

  void feedEscaped(BubbleTracker* bt, State *state);

  void correct(OrdinalGrid<float> *distance);

  std::vector<Particle> getParticles() const;
private:

  void insertParticle(glm::vec3 pos, float radius);

  glm::vec3 jitterCoordinate(glm::vec3 coord);

  void resetParticleCount();

  static constexpr float MAX_RADUIS = 0.5f;
  static constexpr float INTERFACE_OFFSET = 3.0f;

  unsigned particlesPerCell;

  unsigned w, h, d;
  std::vector<Particle*> *interfaceParticles;
  std::stack<Particle*> *deadParticles;
  std::vector<Particle*> *escapedParticles;
  Grid<float> *corrPlus, *corrMinus;
  Grid<unsigned> *particleCount;
};
