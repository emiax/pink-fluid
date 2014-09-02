#pragma once

template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
class State;
struct PressureSolver;
struct VelocityGrid;
class ParticleTracker;

#include <glm/glm.hpp>
#include <util.h>

class Simulator{
public:
  Simulator(State *sf, State *st, float scale = 1.0f);
  ~Simulator();

  void step(State * const readFrom, State* writeTo, float dt);
  void step(float dt);

  // advection
  void advect(State const * readFrom, State * writeTo, float dt);

  // ext. forces
  void applyGravity(State *state, glm::vec2 g, float deltaT);

  // pressure
  void calculateDivergence(State const* readFrom, OrdinalGrid<float> *toDivergenceGrid);
  
  void gradientSubtraction(State *state, float dt);
  void extrapolateVelocity(State *stateFrom, State *stateTo);

  OrdinalGrid<double>* resetPressureGrid();
  OrdinalGrid<float>* getDivergenceGrid();  

  glm::vec2 maxVelocity(VelocityGrid const *const velocity);
  float calculateDeltaT(glm::vec2 maxV, glm::vec2 gravity);
  float getDeltaT();

private:
  void initializeExtrapolation(State *stateFrom);
  unsigned int w,h;
  State *stateFrom, *stateTo;
  OrdinalGrid<float> *divergenceGrid;
  OrdinalGrid<double> *pressureGridFrom, *pressureGridTo;
  OrdinalGrid<float> *levelSetCorrectionGrid;
  PressureSolver *pressureSolver, *jacobiSolver;
  float deltaT;
  float gridSize;

  static constexpr unsigned int PARTICLES_PER_CELL = 16;
  ParticleTracker *pTracker;
};
