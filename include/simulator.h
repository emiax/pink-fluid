template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
class State;

#include <glm/glm.hpp>

class Simulator {
public:
  Simulator(State *statePing, State *statePong);
  ~Simulator();

  void step(State * const readFrom, State* writeTo, float dt);
  void step(float dt);

  // advection
  glm::vec2 backTrack(State const * readFrom, int i,int j, float dt); 
  void advect(State const * readFrom, State * writeTo, float dt);

  // ext. forces
  void applyGravity(const OrdinalGrid<float> *velocityGrid, glm::vec2 g);

  // pressure
  void calculateDivergence(State const* readFrom, OrdinalGrid<float> *toDivergenceGrid);
  void jacobiIteration(unsigned int nIterations);
  void gradientSubtraction(const OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid);

  OrdinalGrid<double>* resetPressureGrid();

private:
  unsigned int w,h;
  State *stateFrom, *stateTo;
  OrdinalGrid<float> *divergenceGrid;
  OrdinalGrid<double> *pressureGrid;
};
