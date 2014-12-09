#pragma once
template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
class State;
struct PressureSolver;
struct VelocityGrid;
class ParticleTracker;
class BubbleTracker;

#include <glm/glm.hpp>
#include <util.h>
#include <vector>
#include <bubble.h>

class Simulator{
public:
  Simulator(const State& initialState, float scale = 1.0f);
  ~Simulator();

  void addBubbles(std::vector<Bubble>& bubbles);
  void step(float dt);
  State* getCurrentState();

  // advection
  // glm::vec3 backTrackU(State const * readFrom, GridCoordinate x, float dt);
  // glm::vec3 backTrackV(State const * readFrom, GridCoordinate x, float dt);
  // glm::vec3 backTrackW(State const * readFrom, GridCoordinate x, float dt);
  // glm::vec3 backTrackMid(State const * readFrom, GridCoordinate x, float dt);
  void advect(State const * readFrom, State * writeTo, float dt);

  // ext. forces
  void applyGravity(State *state, glm::vec3 g, float deltaT);

  // pressure
  void calculateNegativeDivergence(State const* readFrom, OrdinalGrid<float> *toDivergenceGrid);
  
  void gradientSubtraction(State *state, float dt);
  void initializeExtrapolation(State *stateFrom);
  void extrapolateVelocity(State *stateFrom, State *stateTo);

  OrdinalGrid<double>* resetPressureGrid();
  OrdinalGrid<float>* getDivergenceGrid();  

  glm::vec3 maxVelocity(VelocityGrid const *const velocity);
  float calculateDeltaT(glm::vec3 maxV, glm::vec3 gravity);
  float getDeltaT();
  const BubbleTracker* const getBubbleTracker() const;

private:
  unsigned int w,h,d;
  State *stateFrom, *stateTo;
  OrdinalGrid<float> *divergenceGrid;
  OrdinalGrid<double> *pressureGridFrom, *pressureGridTo;
  PressureSolver *pressureSolver, *jacobiSolver;
  float deltaT;
  float gridSize;

  static constexpr unsigned int PARTICLES_PER_CELL = 64;
  ParticleTracker *pTracker;
  BubbleTracker *bTracker;
};
