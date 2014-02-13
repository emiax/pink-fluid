#pragma once
template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
class State;
class VelocityGrid;
#include <glm/glm.hpp>

class Simulator {
public:
  Simulator(State *sf, State *st, float scale = 1.0f);
  ~Simulator();

  void step(State * const readFrom, State* writeTo, float dt);
  void step(float dt);

  // advection
  glm::vec2 backTrackU(State const * readFrom, int i,int j, float dt);
  glm::vec2 backTrackV(State const * readFrom, int i,int j, float dt);
  glm::vec2 backTrackMid(State const * readFrom, int i,int j, float dt);
  void advect(State const * readFrom, State * writeTo, float dt);

  // ext. forces
  void applyGravity(const OrdinalGrid<float> *velocityGrid, glm::vec2 g);

  // pressure
  void calculateDivergence(State const* readFrom, OrdinalGrid<float> *toDivergenceGrid);
  void jacobiIteration(State const* readFrom, unsigned int nIterations);
  void copyBoundaries(State const* readFrom, State* writeTo);

  void gradientSubtraction(State *state, float dt);

  OrdinalGrid<double>* resetPressureGrid();
  
  glm::vec2 maxVelocity(VelocityGrid const *const velocity);
  float calculateDeltaT(glm::vec2 maxV, glm::vec2 gravity);
  float getDeltaT();
  //  void setBoundaries(Grid<bool> *boundaryGrid);


private:
  unsigned int w,h;
  State *stateFrom, *stateTo;
  OrdinalGrid<float> *divergenceGrid;
  OrdinalGrid<double> *pressureGrid;
  float deltaT;
  float gridSize;
};
